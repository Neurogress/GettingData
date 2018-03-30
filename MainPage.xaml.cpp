//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <sstream>
#include <iomanip>

#include <ppltasks.h>

#include "muse.h"

using namespace GettingData;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Radios;
using namespace Windows::Storage;
using namespace Windows::Security::Cryptography;
using namespace interaxon;
using namespace concurrency;
using namespace Windows::Storage::Streams;


// Refresh rate for the data display on the screen (Hz)
#define REFRESH_RATE 60
// Decimal precision for displayed data
#define DATA_PRECISION 4
int DataFlowCounter = 1;
int RepeatingOperator = 0;
bool isRecording = 0;
Platform::String^ buffertext ="";
int bufferinfilation = 0;
bool TimeOutVar = 0;
Platform::String^ isWrited = "isWrited";
Platform::String^ RecordingEnum;
Platform::String^ CurrentFileName;
Platform::String^ UserName = "default";

MainPage::MainPage() :
    current_data_type_(MuseDataPacketType::EEG),
    my_muse_(nullptr),
	
    enable_data_(true)
{
    InitializeComponent();
    //init_data_type_combobox();

    queue_ui_update();

    manager_ = MuseManagerWindows::get_instance();
    muse_listener_ = std::make_shared<GettingData::MuseListenerWin>(this);
    connection_listener_ = std::make_shared<GettingData::ConnectionListener>(this);
    data_listener_ = std::make_shared<GettingData::DataListener>(this);
    manager_->set_muse_listener(muse_listener_);
    is_bluetooth_enabled_.store(false);
    check_bluetooth_enabled();
}


void MainPage::check_bluetooth_enabled() {
    // This task is async and will update the member variable when it is run.
    create_task(Radio::GetRadiosAsync()).then([this](IVectorView<Radio^>^ radios) {
        for (auto r : radios) {
            if (r->Kind == RadioKind::Bluetooth) {
                if (r->State == RadioState::On) {
                    is_bluetooth_enabled_.store(true);
                }
                else {
                    is_bluetooth_enabled_.store(false);
                }
                break;
            }
        }
    });
}

bool MainPage::is_bluetooth_enabled() {
    // Must call check_bluetooth_enabled first to run
    // an async to check if bluetooth radio is on.
    return is_bluetooth_enabled_.load();
}

// Muse callback methods
void MainPage::muse_list_changed() {
    OutputDebugString(L"MainPage::muse_list_changed\n");
    update_muse_list();
}

void MainPage::receive_connection_packet(const MuseConnectionPacket & packet, const std::shared_ptr<Muse> & muse) {
    model_.set_connection_state(packet.current_connection_state);
    OutputDebugString(L"MainPage::receive_connection_packet\n");

    // The Muse version is only available in the connected state.
    if (packet.current_connection_state == ConnectionState::CONNECTED) {
        auto version = my_muse_->get_muse_version();
        model_.set_version(version->get_firmware_version());
    }
    else {
        model_.set_version("Unknown");
    }

}

void MainPage::receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse) {
    model_.set_values(packet);
    OutputDebugString(L"MainPage::receive_muse_data_packet\n");
}

void MainPage::receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse) {
    model_.set_values(packet);
    OutputDebugString(L"MainPage::receive_artifact_packet\n");
}

void MainPage::refresh_button_clicked(Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    manager_->stop_listening();
    manager_->start_listening();
}

void MainPage::connect_button_clicked(Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto selected_muse = static_cast<Platform::String^>(muse_list_combobox->SelectedItem);
    my_muse_ = get_muse(selected_muse);

    if (nullptr != my_muse_) {
        // Stop listening to after selecting a Muse device to connect to
        manager_->stop_listening();
        model_.clear();
        my_muse_->register_connection_listener(connection_listener_);
        my_muse_->register_data_listener(data_listener_, current_data_type_);
        my_muse_->run_asynchronously();
    }
}

void MainPage::disconnect_button_clicked(Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if (my_muse_ != nullptr) {
        my_muse_->disconnect();
    }
}

void GettingData::MainPage::pause_resume_clicked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
    if (my_muse_ != nullptr) {
        enable_data_ = !enable_data_;
        my_muse_->enable_data_transmission(enable_data_);
    }
}

void MainPage::queue_ui_update() {
    create_task([this]() {
        WaitForSingleObjectEx(GetCurrentThread(), 1000 / REFRESH_RATE, false);
    }).then([this]() {
        Dispatcher->RunAsync(
            Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler([this]()
        {
            update_ui();
        })
            );
    });
}

// Call only from the UI thread
void MainPage::update_ui() {
    if (model_.is_dirty()) {
        double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

        model_.get_buffer(buffer);
	// printing the message about the data being saved
	if (isWrited == "The data is saved")
		command_text->Text = isWrited;
	// beginning the data recording
	if (DataFlowCounter == 0){
		command_text->Text = "Get ready";
		buffertext = Platform::String::Concat(buffertext, "prepare \r\n");
	}
	// writing the tagging
	if ((DataFlowCounter % 100 == 0)&&(DataFlowCounter < 201)&&(RepeatingOperator < 10)&& (isRecording == 1)){
		if ((DataFlowCounter == 100) ){
			command_text->Text = RecordingEnum;
			buffertext = Platform::String::Concat(buffertext, "action \r\n");
		}
		if (DataFlowCounter == 200){
			command_text->Text = "Relax youre hand";
			buffertext = Platform::String::Concat(buffertext, "relax \r\n");
			DataFlowCounter = 1;
			RepeatingOperator ++; 
		}
	}
	else 
		if((RepeatingOperator == 10) && (DataFlowCounter < 201)){
			DataFlowCounter = 201;
			RepeatingOperator = 0;
			isRecording = 0;
			addtext(buffertext, 2, CurrentFileName);
		}
	// writing the string of values
	if (isRecording == 1){
		buffertext = Platform::String::Concat(buffertext, formatData(buffer[0]));
		buffertext = Platform::String::Concat(buffertext, ", ");
		buffertext = Platform::String::Concat(buffertext, formatData(buffer[1]));
		buffertext = Platform::String::Concat(buffertext, ", ");
		buffertext = Platform::String::Concat(buffertext, formatData(buffer[2]));
		buffertext = Platform::String::Concat(buffertext, ", ");
		buffertext = Platform::String::Concat(buffertext, formatData(buffer[3]));
		buffertext = Platform::String::Concat(buffertext, "\r\n");

		bufferinfilation = 0;
			
		DataFlowCounter++;
	}

        line_1_data->Text = formatData(buffer[0]);
	line_2_data->Text = formatData(buffer[1]);
	line_3_data->Text = formatData(buffer[2]);
	line_4_data->Text = formatData(buffer[3]);

        connection_status->Text = Convert::to_platform_string(model_.get_connection_state());
        version->Text = Convert::to_platform_string(model_.get_version());

        model_.clear_dirty_flag();
    }
    queue_ui_update();
}
void MainPage::update_muse_list() {
    auto muses = manager_->get_muses();
    muse_list_combobox->Items->Clear();
    for (auto m : muses) {
        auto name = Convert::to_platform_string(m->get_name());
        muse_list_combobox->Items->Append(name);
    }
    if (muse_list_combobox->Items->Size) {
        muse_list_combobox->SelectedIndex = 0;
    }
}
std::shared_ptr<Muse> MainPage::get_muse(Platform::String^ desired_name) {
    auto muses = manager_->get_muses();
    for (auto m : muses) {
        auto name = Convert::to_platform_string(m->get_name());
        if (name->Equals(desired_name)) {
            return m;
        }
    }
    return nullptr;
}


void MainPage::set_eeg_ui() {
    set_ui_line(line_1_label, "EEG1", line_1_data, true);
    set_ui_line(line_2_label, "EEG2", line_2_data, true);
    set_ui_line(line_3_label, "EEG3", line_3_data, true);
    set_ui_line(line_4_label, "EEG4", line_4_data, true);
}

void MainPage::set_ui_line(TextBlock^ label, String^ name, TextBlock^ data, bool visible) {
    label->Text = name;
    data->Text = "0.0";

    Windows::UI::Xaml::Visibility visibility = (visible)
        ? Windows::UI::Xaml::Visibility::Visible
        : Windows::UI::Xaml::Visibility::Collapsed;
    label->Visibility = visibility;
    data->Visibility = visibility;
}
Platform::String^ MainPage::formatData(double data) const
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(DATA_PRECISION) << data;
    return Convert::to_platform_string(ss.str());
}




// Creating 8 files in the directory fo signal data recording
void GettingData::MainPage::file_create_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	StorageFolder^ storageFolder = ApplicationData::Current->LocalFolder;

	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "fistrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //fist
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "okrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //ок
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "vrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //v
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "chiprecording.txt"), CreationCollisionOption::GenerateUniqueName)); //pinch
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "cylindrrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //cylindrical
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "bigrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //thumb up
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "pointrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //forefinger
	concurrency::create_task(storageFolder->CreateFileAsync(String::Concat(UserName, "rotrecording.txt"), CreationCollisionOption::GenerateUniqueName)); //rotation in the carpal joint

	foldername->Text = storageFolder->Path;
}

// Name typing
void GettingData::MainPage::InputName_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	UserName = foldername->Text;
}

// Writing the string of values in the correspondent text file
void MainPage::addtext(Platform::String^ text, int newline, Platform::String^ FileName)
{	
	if (newline == 2)
		text = Platform::String::Concat(text, "\r\n");
	if (newline == 1)
		text = Platform::String::Concat(text, ", ");

	StorageFolder^ storageFolder = ApplicationData::Current->LocalFolder;

	create_task(storageFolder->GetFileAsync(FileName)).then([text](StorageFile^ sampleFile)
	{
		create_task(FileIO::AppendTextAsync(sampleFile, text));
	}).then([]()
	{
		isWrited = buffertext ="The data is saved";
	});
}

// Guiding instructions to the corresponding action buttons for the brain data capturing
void GettingData::MainPage::record_fist_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Clench your fist";
	CurrentFileName = (String::Concat(UserName, "fistrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";

}


void GettingData::MainPage::record_ok_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Perform an OK gesture";
	CurrentFileName = (String::Concat(UserName, "okrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


void GettingData::MainPage::record_ship_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Perform a pinch grip";
	CurrentFileName = (String::Concat(UserName, "chiprecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


void GettingData::MainPage::record_cilindr_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Perform a cylindrical grip";
	CurrentFileName = (String::Concat(UserName, "cylindrrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


void GettingData::MainPage::record_big_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Perform a thumb up";
	CurrentFileName = (String::Concat(UserName, "bigrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


void GettingData::MainPage::record_point_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Perform a forefinger";
	CurrentFileName = (String::Concat(UserName, "pointrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


void GettingData::MainPage::record_rotation_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Rotate your wrist";
	CurrentFileName = (String::Concat(UserName, "rotrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


void GettingData::MainPage::record_v_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RecordingEnum = "Perform the v gesture";
	CurrentFileName = (String::Concat(UserName, "vrecording.txt"));
	DataFlowCounter = 0;
	isRecording = 1;
	isWrited = "preparing data";
}


