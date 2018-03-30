//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "DataModel.h"

#include "muse.h"

using namespace interaxon::bridge;

namespace GettingData
{
    class MuseListenerWin;
    class DataListener;
    class ConnectionListener;


    /// <summary>
    /// MainPage contains the UI of the example application and manages the
    /// connection to the Muse headband.  Data is received from the headband
    /// via a listener callback.  Received data is stored in the DataModel
    /// until the next UI update.
    /// </summary>
    public ref class MainPage sealed
    {
    public:

        /// Constructs a MainPage
        MainPage();

    private:

        //////////////////////////////////////////////////////
        /// Initialization functions

        /// Initializes the combobox for selecting which type of
        /// data you want to receive.
        void init_data_type_combobox();

        /// Adds a name to the combobox for selecting which type of
        /// data you want to receive and associates a packet type with
        /// the name.
        void add_type(Platform::String^ name, MuseDataPacketType type);

        //////////////////////////////////////////////////////
        /// Bluetooth functions

        /// Creates an asynchronous task to check if there are
        /// any Bluetooth radios that are currently turned on.
        /// The result is accessible from is_bluetooth_enabled.
        void check_bluetooth_enabled();

        /// Returns the last result from the call to check_bluetooth_enabled().
        /// Returns true if a Bluetooth radio is on, false if all Bluetooth
        /// radios are off.
        bool is_bluetooth_enabled();


        //////////////////////////////////////////////////////
        /// Listener callback functions.
        ///

        /// muse_list_changed is called whenever a new headband is detected that
        /// can be connected to or an existing headband is detected as no longer
        /// available.  You must be "listening" in order to receive these callbacks.
        /// To start listening, call MuseManager::start_listening();
        ///
        /// Once you have received the callback, you can get the available
        /// headbands by calling MuseManager::get_muses();
        void muse_list_changed();

        /// You receive a connection packet whenever the state of a headband changes
        /// between DISCONNECTED, CONNECTING and CONNECTED.
        ///
        /// \param packet Contains the prior and new connection states.
        /// \param muse   The muse that sent the connection packet.
        void receive_connection_packet(const MuseConnectionPacket & packet, const std::shared_ptr<Muse> & muse);

        /// Most data from the headband, including raw EEG values, is received as
        /// MuseDataPackets.  While this example only illustrates one packet type
        /// at a time, it is possible to receive multiple data types at the same time.
        /// You can use MuseDataPacket::packet_type();
        ///
        /// \param packet The data packet.
        /// \param muse   The muse that sent the data packet.
        void receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse);

        /// Artifacts are boolean values that are derived from the EEG data
        /// such as eye blinks or clenching of the jaw.
        ///
        /// \param packet The artifact packet.
        /// \param muse   The muse that sent the artifact packet.
        void receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse);


        //////////////////////////////////////////////////////
        ///  UI Callbacks

        /// Called when the user presses the "Refresh" button.
        /// This calls MuseManager::start_listening to start receiving muse_list_changed
        /// callbacks.
        void refresh_button_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        /// Called when the user presses the "Connect" button.
        /// This will initiate a connection to the Muse headband selected in the
        /// top drop down.
        void connect_button_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        /// Called when the user presses the "Disconnect" button.
        /// This will disconnect from the Muse headband that is currently connected.
        void disconnect_button_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        /// Called when the user presses the "Pause/Resume" button
        /// This will toggle between pause and resume data transmission.
        void pause_resume_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        /// Called when the user selects a different data type to display from
        /// the center combobox.
        void data_type_selection_changed(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);




        //////////////////////////////////////////////////////
        ///  Helper functions for the UI

        /// EEG Data is received at a much higher rate (220 or 256Hz) than it is
        /// necessary to update the UI.  This function will ask for a UI update
        /// at a more reasonable rate (60Hz).
        void queue_ui_update();

        /// Handles a single update of the screen by transferring the data from
        /// the DataModel to the UI elements.
        ///
        /// Called from queue_ui_update on the Core Dispatcher.
        void update_ui();


        /// Updates the combobox at the top of the screen with the list of
        /// available Muse headbands.  Called when a muse_list_changed callback
        /// is received.
        void update_muse_list();

        /// Finds the Muse with the specified name in the list of Muses returned
        /// from MuseManager.
        std::shared_ptr<Muse> get_muse(Platform::String^ name);

        /// Unregisters the current MuseDataListener and registers a listener for
        /// the new data type.
        ///
        /// \param type The MuseDataPacketType you want to display.
        void change_data_type(MuseDataPacketType type);

        /// Update the titles of the displayed data to reflect the type of packet
        /// that we are receiving; Accelerometer, Battery, DrlRef, EEG, Gyro or
        /// Artifact respectively.  EEG is used for all EEG derived values like
        /// ALPHA_ABSOLUTE, BETA_RELATIVE, HSI_PRECISION, etc.
        void set_accel_ui();
        void set_battery_ui();
        void set_drl_ref_ui();
        void set_eeg_ui();
        void set_gyro_ui();
        void set_artifacts_ui();

        /// Sets the text on a single line to the specified value.  Hides or
        /// shows the line as appropriate.
        ///
        /// \param label The TextBlock that defines the type of data.
        /// \param name  The value to set as the Text in "label".
        /// \param data  The TextBlock that defines the value of the data.
        ///              This will be initialized to 0.0
        /// \param visible True if the line should be shown, false if it should
        ///                be hidden.
        void set_ui_line(::Windows::UI::Xaml::Controls::TextBlock^ label,
                         ::Platform::String^ name,
                         ::Windows::UI::Xaml::Controls::TextBlock^ data,
                         bool visible);

        /// Formats a double value to a String with the desired number of
        /// decimal places.
        Platform::String^ formatData(double data) const;

		void addtext(Platform::String ^ text, int newline, Platform::String ^ FileName);

		void addtext(Platform::String ^ text, int newline);

		

		




        //////////////////////////////////////////////////////
        ///  Variables

        // A reference to the MuseManager instance.
        std::shared_ptr<interaxon::bridge::MuseManagerWindows> manager_;

        /// The individual listener interfaces in LibMuse are abstract classes.
        /// The following classes are defined at the end of the file.  Each
        /// inner class implements a different interface and forwards the
        /// information received back to this MainPage object.
        friend class ConnectionListener;
        friend class MuseListenerWin;
        friend class DataListener;

        std::shared_ptr<MuseListenerWin> muse_listener_;
        std::shared_ptr<ConnectionListener> connection_listener_;
        std::shared_ptr<DataListener> data_listener_;

        /// A reference to the Muse object that we are currently connected to.
        /// This is useful so we know which Muse to disconnect.
        std::shared_ptr<Muse> my_muse_;

        /// The current type of data that we are listening to.  This is set
        /// from the combobox in the middle of the screen.
        MuseDataPacketType current_data_type_;

        /// The Data Model object that is used to collect and store the data
        /// we receive.
        DataModel model_;

        /// The last result returned from the check_bluetooth_enabled function.
        std::atomic_bool is_bluetooth_enabled_;

        /// A map for getting the MuseDataPacketType from the name in the
        /// data type combobox.
        Platform::Collections::Map<Platform::String^, int> name_to_type_map_;

        /// Toogle for pause/resume data transmission.
        bool enable_data_;
		void file_create_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void command_text_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_fist_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_ok_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_ship_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_cilindr_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_big_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_point_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_rotation_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void record_v_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void InputName_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
};


    //////////////////////////////////////////////////////
    ///  Listener Implementation Classes

    class MuseListenerWin : public interaxon::bridge::MuseListener {
    public:
        MuseListenerWin(MainPage^ mp) : main_page_(mp) {}
        void muse_list_changed() override {
            main_page_->muse_list_changed();
        }
    private:
        MainPage^ main_page_;
    };

    class ConnectionListener : public interaxon::bridge::MuseConnectionListener {
    public:
        ConnectionListener(MainPage^ mp) : main_page_(mp) {}
        void receive_muse_connection_packet(const MuseConnectionPacket & packet, const std::shared_ptr<Muse> & muse) override {
            main_page_->receive_connection_packet(packet, muse);
        }
    private:
        MainPage^ main_page_;
    };

    class DataListener : public MuseDataListener {
    public:
        DataListener(MainPage^ mp) : main_page_(mp) {}
        void receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse) override {
            main_page_->receive_muse_data_packet(packet, muse);
        }

        void receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse) override {
            main_page_->receive_muse_artifact_packet(packet, muse);
        }
    private:
        MainPage^ main_page_;
    };
}
