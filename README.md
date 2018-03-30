# GettingData

## Description
This code reads data from a brain activity via the Muse neurointerface. 

The code uses Muse libraries in order to correctly work with the neurointerface. Muse examples and libraries can be found here: http://developer.choosemuse.com/sdk/developer-sdk-windows/getting-started-with-libmuse-windows. 

The code is written using the Visual Studio IDE. The installation of a package is done following the standard uwp pproject guidelines. Here are a couple of links for working with uwp project:
1. https://docs.microsoft.com/en-us/windows/uwp/get-started/whats-a-uwp
2. https://docs.microsoft.com/en-us/windows/uwp/packaging/packaging-uwp-apps


## Manual to capture brain activity data using the GettingData app
* Refer the following steps to the image below.
1. Start the program. 
2. Press the 'Refresh' button (1).
3. Select available neurointerface from the menu (2). 
4. Press the 'Connect' button (3).
5. Enter the name into the field (4). That name will be used in the files created for the data capturing.
6. Press the 'input name' button  (5).
7. Press the 'create file' (6). There will be a directory path in the field (4) where the written data will be stored. 
8. Select the required gesture by pressing one of the buttons (8).
9. Follow instructions from the field (7). 

![Manual for GettingData brain activity capturing](https://raw.githubusercontent.com/Neurogress/GettingData/master/img/manual.jpg)

As a result, we receive 8 text files with the data which is written in the following way: 
"action name
a line with four values (100 lines total)"
10 blocks per file corresponding to each repetition.

Subsequently, the output data will be used to identify patterns and interactions between actions and changes in the neurointerface indications.
