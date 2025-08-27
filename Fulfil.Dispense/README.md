# Fulfil.Dispense

The only information guaranteed to be  up-to-date is the **Starting the Mars API** section. Any information that precedes the **Depreciation Warning** section of the readme is valid. Other up-to-date information can be found on the [CV Command Guide](https://docs.google.com/document/d/14k9Jj3RgEqf9b27t3sbYC5RS6eEuN8hgaCXUnPTA268/edit?pli=1#heading=h.5wo37tlgfgcn).

## Deploying

Because building giant ARM64 images in Github Actions is nearly impossible, we instead use a dedicated CI build box in Whisman that can
be used to replicate Github's current AMD images that are successfully built as a CI check on Pull requests.

### 1. Login to build box

`ssh fulfil@clip-tb.whisman.fulfil.ai` while connected to VPN.

Ask someone on CV for access if needed.

### 2. Build and push a Production ARM image to GCP Artifact Registry

From the build box, run `bash /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/scripts/dab-push-latest.sh`, it will prompt you for the branch and facility to build against.

### 3a. Deploy latest facility build to entire facility

All machines point to their respective `main:<FACILITY>` docker images in Google by default, but each production machine
will never attempt to update it's image unless told to do so in vanilla docker fashion.

To mass-deploy a facility-specific tagged image to an entire facility, run:

`cd Fulfil.Dispense && bash scripts/dab-update-facility.sh` from _your local machine over VPN_, the Whisman build box is not allowed to connect to other facilities directly.  This option forces you to use `main:<FACILITY>` docker images only!

### 3b. Deploy custom builds per machine

For niche production deployments, run `bash /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/scripts/dab-pull-latest.sh` on the target
machine to deploy and restart services on. This script also lets you select custom GIT branches.


## Logging

All Depthcam / Dispense / TrayCount / (all running Docker containers really) can be searched in [Grafana Loki](https://grafana.fulfil-api.com/explore?schemaVersion=1&panes=%7B%22pwc%22:%7B%22datasource%22:%22DOKYKGqV2%22,%22queries%22:%5B%7B%22refId%22:%22A%22,%22expr%22:%22%7Blocation%3D%5C%22pioneer%5C%22,%20name%3D%5C%22depthcam%5C%22%7D%20%7C%3D%20%60%60%22,%22queryType%22:%22range%22,%22datasource%22:%7B%22type%22:%22loki%22,%22uid%22:%22DOKYKGqV2%22%7D,%22editorMode%22:%22builder%22%7D%5D,%22range%22:%7B%22from%22:%22now-5m%22,%22to%22:%22now%22%7D%7D%7D&orgId=1).  You can narrow down the logs you want by filtering on these relevent fields (Grafana's value auto-complete will help you after entering these key names):

- `location`: The facility the service is running in
- `name`: The name of the service or container (`depthcam` or `traycount`)
- `machine`: The name of the machine running the service, ex. `P2-DAB`

Grafana log scraping config can be found in the [Alloy folder](../alloy) while machine-specific env vars can be found remotely at `home/fulfil/code/Fulfil.ComputerVision/alloy/local.dev`.

## Starting the Mars API

Log on as the fulfil user (or run the command from fulfil userspace). Run the simple_startup script to cycle 
the services. G-Dabs are run on `.83` and F-Dabs are run off `.88`. 
```
# ssh fulfil@10.10.103.83 or 88
cd code
./simple_start.sh
# kills all running shells related to depthcams 
./simple_start.sh kill
```
Unless you are starting right after a power cycle, I recommend checking the depthcam session first (see below). 
You may only need to scroll up to the last command and restart the api from there. If you are using the script,
should start the sessions only if they do not exist. Validate that cameras start properly by logging onto the depthcam 
tmux session and watching until the connection is established (be careful not to quit the session while detaching).
```
tmux a -t depthcam
# should see something like the following lines on clean startup
[10:00:52] [0ms] [-info-] [3403] Start bay manager...
[10:00:52] [0ms] [-info-] [3641] Starting bay 1: FD1
[10:00:52] [0ms] [-info-] [3642] Starting bay 0: FD2
[10:00:52] [0ms] [-info-] [3643] search for connection...
[10:00:52] [0ms] [-info-] [3644] search for connection...
[10:00:53] [351ms] [-info-] [3644] connection established
[10:00:53] [140ms] [-info-] [3643] connection established
```
If the api fails to start, run `rs-fw-update -l` to list out all four cameras and ensure that they are allowing basic 
queries. If that command fails, you may need to power cycle the board.

## Depreciation Warning

THIS README IS OUTDATED AND NEEDS FIXING. DO NOT READ THROUGH IT BEFORE CHECKING WITH SENIOR CV TEAM ENGINEERS. 

CV Team onboarding documentation for now can be found at this link: https://docs.google.com/document/d/15cguOQNcNPB-vgwU3ZplAzT1tSVVaBIE?rtpof=true&authuser=steve.burke%40fulfil.ai&usp=drive_fs

## Integration
This project follows the guidelines that are specified int the [C++ styleguide]( https://github.com/Fulfil0518/styleguides/blob/master/cpp-styleguide.md). To integrate Fulfil.Dispense into a project, add this repository to the project as a sub module. In the main CMakeLists.txt file of your project, add the line `add_subdirectory(Fulfil.Dispense)` and make sure to link the DepthCam library to your target with `target_link_libraries(target_name Fulfil.Dispense)` in src/CMakeLists.txt as well as app/CMakeLists.txt of the project you are adding it to.

## Dependencies
* Fulfil.DepthCam v3.0
* Fulfil.CPPUtils v3.0.1
* OpenCV 4.1.1 (with Aruco)
* Realsense2
* GooglTest (pulls in master branch automatically)

## CMake Options
By default the logging directory is set to `<CMake project root>/logs` and the configs directory is set to `<CMake project root>/configs`. If you are
building on the machine that you plan to run on, this is most likely the correct configuration. If you want to build for another machine that will have
a different default location for these directories run cmake with the `FULFIL_LOG_DIR` and `FULFIL_INI_DIR` flags respectively. For example, if 
you want to change the defaults for both directories: 
```
 cmake -DFULFIL_INI_DIR=/new/ini/path -DFULFIL_LOG_DIR=/new/log/path ..
 make
```
**WARNING** Running this defines the build tree. If you want to reset the build tree to system defaults without deleting the entire build directory run:
```
cmake -DRESET_FULFIL_DEFAULTS=ON ..
```
After initially running with either flag set, you can run `cmake ..` with out the defined directory location changing. 
You can also redefine the paths as many times as you like without using `RESET_FULFIL_DEFAULTS` first by manually 
running `FULFIL_LOG_DIR` and `FULFIL_INI_DIR`.

## Operation

This repository is run as one of three services on the Single Board Computers (SBCs) that control the depthcams. The service file `docs/depthcam.service` is required to be moved into the `/etc/systemd/system` folder or equivalent. The other two services required to run the depthcam API are `bg_redis_worker.service` and `tray_count.service`, located in the TrayCountAPI repository. 

The depthcam service requires the other two services to be up and running in order to start. Therefore to set up the depthcam API, enter the following command: `sudo systemctl start tray_count.service && sudo systemctl start bg_redis_worker.service && sudo systemctl start depthcam.service`. Alternatively, execute the `resetdc` bash file. Each service opens a tmux session which runs the requisite code. The depthcam service runs `main` for this repo. 

In addition to the three services described, there is also an automatic disk cleaning utility. This is implemented with `disk_clean_util.py`. This script is executed on an interval with a cron job on each SBC. `disk_clean_util` takes three arguments: `disk_clean_util.py <days of drop images to keep> <days of tray images to keep> <days of videos to keep>`. All MP4 files used to produce the videos are deleted immediately when the script is run. An example of a cron job is shown here, where we want to run the script once a day at 12:30pm, and delete all drop images from more than 2 days ago, all tray images from more than 5 days ago, all videos from more than 1 day ago. 

```
30 12 * * * ~/code/Fulfil.ComputerVision/Fulfil.Dispense/disk_clean_util.py 2 5 1
```


## Structure

* [Executables](#executables)
    * [main](#main)
    * [Camera live feed](#camera-live-feed)
    * [Auto tray calibration](#auto-tray-calibration)
    * [Dispense test client](#dispense-test-client)
* [Libs](#libs)
    * [Depthcam](#depthcam)
    * [MongoCPP](#mongocpp)
    * [CPPUtils](#cpputils)
* [Configs](#configs)
    * [AGX Specific](#agx-specific)
    * [LFB Configs](#lfb-configs)

### Executables
This repo contains a few independent executables other than the primary one `main`. Some noteable ones and their purpose is described in this section: 

#### main
Entry point into the dispense routine.

#### Camera live feed
Used to help align the depthcams if they have been adjusted, knocked off position, or otherwise needed. Creates a live view from a specified camera. Also contains view-lines used to help align with certain features on the VLS.

#### Auto tray calibration
Used to generate tray calibration data for various tray positions relative to the tray camera. This information is used by the Tray Count API. 

#### Dispense test client
Mimics the factory control by sending mock requests in JSON format using TCP over localhost. Used to test changes locally to verify certain functionality.


### Libs
The dispense code contains many dependencies. Each of these are their own independent repositories that can be found in the Fulfil codebase. They are run from the `libs` folder. When updating the code, they must be updated from there as well in order for the changes to take effect. For instance, if we were to update `Fulfil.Depthcam`, we need to enter `Fulfil.Dispense/libs/Fulfil.Depthcam`, and run git update from there. Each of these repos contain their own READMEs, but a brief overview of each is given below: 

#### Depthcam
Used to integrate with various depthcam functionalities such as visulization tools or pointcloud coordinate conversions.

#### MongoCPP
Custom library used to interface with Factory Mongo.

#### CPPUtils
Miscellaneous library with various C++ utlities. Most noteable ones are tools for working with eigen matrices, logging, and TCP networking. 

### Configs

Note: a number of configuration parameters are stored in Factory Mongo. This number may or may not decrease as we make changes to the repo.

#### AGX Specific
`configs/AGX_specific_main.ini` is not present in this repo. It must be copied on each individual machine from `docs/AGX_specific_main.ini` and modified accordingly. It contains parameters specific to each VLS, such as depthcam serial numbers and IP addresses.

#### LFB Configs
`configs/LFBx_config.ini` where x = 1, 1b, or 2, contain configs specific for dispensing to each generation of LFBs. 


## Design
* [Bays](#bays)
    * [Defined Interfaces](#defined-interfaces)
        * [Template Classes](#template-classes)
        * [Bay Runner](#bay-runner)
        * [Bay Runner Factory](#bay-runner-factory)
        * [Bay Parser](#bay-parser)
    * [Creating Bays](#creating-bays)
* [Commands](#commands)
    * [Dispense Request](#dispense-request)
    * [Dispense Response](#dispense-response)
    * [Dispense Command](#dispense-command)
    * [Nop](#nop)
    * [Pre Drop](#pre-drop)
    * [Stop](#stop)
    * [Parsing Commands](#parsing)
* [Drop](#drop)
    * [Using Drop Manager](#using-drop-manager)
* [Dispense](#dispense)
    * [Image Persistence](#image-persistence)
    * [Using Dispense Manager](#using-dispense-manager)
* [Visualization](#visualization)
    * [ABIS](#abis) 
    * [Live DepthCam Images](#live-depthcam-images)
* [Running Dispense Firmware](#running-dispense-firmware)
    * [Live Testing Dispense Firmware](#live-testing-dispense)
    
### Bays
The purpose of bays is to create a generic class that will provide all of the functionality for having multiple bays running on the same computer. It handles all of the complexity that comes along with multi threaded coding and provides easily hot swappable functionality for flexibility in the future.

#### Defined Interfaces
To allow for BayManger (the core class in this section of the code) to be generalizable, there are four interfaces provided that allow the implementation of necessary components to be separate from the BayManager.

##### Template Classes
All of the interfaces take in a template class called Sensor. For all of these interfaces to work together, they must take in the same type. You will not be able to provide a sensor manager where the template class is an int if the bay runner has the template class as a Session. Additionally the template class is called Sensor but it doesn't actually have to reference any sort of sensor. It can simply be an int.

##### Bay Runner
BayRunner defines a simple interface that outlines the ability to start the execution of something and some other functions to help with flexibility. What the class that implements BayRunner does is entirely up to whoever implements it. There are no restrictions on what it can do.

##### Bay Runner Factory
BayRunner factory essentially serves to define a way to construct a BayRunner. Baked into the BayManager class is the idea of having a sensor that goes with each Bay. For this reason, the BayRunnerFactory provides methods to create BayRunners by providing a sensor and by providing no sensor for when the sensor couldn't be found. 

##### Bay Parser
The bay parser requires classes that implement it to provide a function that takes in the generic sensor parameter and return the integer identifier of the bay for that parameter. 

#### Creating Bays
The bay manager has all the functionality to run whatever number of bays in separate threads. All you have to do is pass it the implementation of the interfaces and you are good to go. The following code example will illustrate this and the code can be run with bay_manager_example.cpp.

```c++
// Creating example of BayRunner
class SpyBayRunner : public fulfil::dispense::bays::BayRunner,
    public std::enable_shared_from_this<SpyBayRunner>
{
 private:
  int sensor;
  int bay_num;
 public:
  SpyBayRunner(int baynum, int sensor)
  {
    this->bay_num = baynum;
    this->sensor = sensor;
  }

  explicit SpyBayRunner(int baynum)
  {
    this->bay_num = baynum;
    this->sensor = -1;
  }

  void start() override
  {
    std::cout << "Bay " << this->bay_num << " ran bay runner " << this->sensor << " ran" << std::endl;
  }

  void bind_delegates() override
  {
    //Not going to do anything here.
  }

  std::shared_ptr<BayRunner> get_shared_ptr() override
  {
    return this->shared_from_this();
  }
};

// Creating an example Bay Runner Factory
class SpyBayRunnerFactory : public fulfil::dispense::bays::BayRunnerFactory<int>
{
  std::shared_ptr<fulfil::dispense::bays::BayRunner> create(int bay_num, int sensor) override
  {
    return std::make_shared<SpyBayRunner>(bay_num, sensor);
  }
  std::shared_ptr<fulfil::dispense::bays::BayRunner> create_empty(int bay_num) override
  {
    return std::make_shared<SpyBayRunner>(bay_num);
  }
};

// Creating an example sensor manager
class SpySensorManager : public fulfil::dispense::bays::SensorManager<int>
{
  std::shared_ptr<std::vector<int>> get_connected_sensors() override
  {
    std::shared_ptr<std::vector<int>> connected = std::make_shared<std::vector<int>>();
    // Skipping the 0 index to show that the constructor without a sensor is called
    for(int i = 1; i < 4; i++)
    {
      connected->push_back(i);
    }
    return connected;
  }
};

// Creating an example bay parser
class SpyBayParser : public fulfil::dispense::bays::BayParser<int>
{
  int get_bay(int sensor) override
  {
    return sensor;
  }
};


int main(int argc, char** argv)
{
  // Instantiating all of the required stuff for the bay manager.
  std::shared_ptr<SpySensorManager> sensor_manager = std::make_shared<SpySensorManager>();
  std::shared_ptr<SpyBayRunnerFactory> runner_factory = std::make_shared<SpyBayRunnerFactory>();
  std::shared_ptr<SpyBayParser> bay_parser = std::make_shared<SpyBayParser>();

  // Creating a bay manager where sensors are ints to match the previously defined classes.
  std::shared_ptr<fulfil::dispense::bays::BayManager<int>> manager = std::make_shared<fulfil::dispense::bays::BayManager<int>>(sensor_manager, runner_factory, bay_parser, 4);
  manager->start();
  return 0;
}
```

### Commands
The depth cam needs to be able to process a variety of requests in the dispense process. There are functional requirements for all requests and responses defined by the DispenseRequest interface and the DispenseResponse interface and there are some other helper interfaces to execute commands and outline what commands are available.

#### Dispense Request
Each command must have the ability to execute itself (this was a design decision that helped reduce complexity in the code). In order to execute the order, it is designed to make use of the DispenseRequestDelegate. The idea here is that you can call execute on every command and then, using double dispatch, the command is able to call the right function for executing on its delegate. The delegate is meant to be an all encompassing delegate that can execute for every command. There is also a command id with each request as well as a debug string. Right now command and request can be mixed up in the code naming and documentation. In general a request should be something received from the network and a command is only once it has been parsed and is associate with a specific function such as pre drop calculations.

#### Dispense Response 
Dispense Response essentially is an adapter for the SocketResponse. There is information that is encoded that is generic for all requests, and then there are specific results for different types of requests. The DispenseResponse class provides implementations for the SocketResponse functions and requires classes that inherit from it to implement another subset that are used by DispenseResponse to respond.

#### Dispense Command
Dispense command is an enum that enumerates all different types of commands that the depth camera handles. This can be expanded upon very easily.

#### Nop
The nop command is sent by the C# side for a reason that I don't remember. Essentially this request and response pair don't do anything but exist to work well with the current structure of the program. The nop request is put on the processing queue once and then the response doesn't actually trigger a response to be sent ot the C# server.

#### Pre Drop
The pre drop request handles searching for the drop zone for the item specified by the request. There is a helper class PreDropDetails which contains all of the information necessary for finding the optimal location for dropping an item. The pre drop request is currently re-added to the queue until a stop command is received to always try to send more updated information to the C# server. If the delegate is still valid, the request passes the details to the delegate to process and find the drop zone, it then gets the result which contains the absolute x position of the rover, absolute y position of the extend, and the absolute z position of the platform on the rover (NOTE: the values calculated here have never been tested and will need tweaking on the new VLAD). The response encodes all of that information into json and provides that as the payload to be sent to the C# server.

#### Stop
When a stop command is received, it removes all commands from the processing queue that have and ID that matches the stop command. Practically this is used to stop the constant update from the Pre Drop Command. The stop response, similarly to the nop response, does not actually trigger a response to be sent to the C# server.

#### Parsing
There are two (technically three) stages of parsing that a request will go through before it is converted into a request class. 
1. The payload is parsed into json and the json value where the type of request is stored is extracted and that integer value is converted to an enum value for the type of request. 
1. After the type is determined it parses any json related to the request (not all requests have json payload so this doesn't always happen)
1. After the payload is parsed, it creates the appropriate request object.

This is managed by the DispenseCommandParser which draws on other classes to handle the sub steps.

### Drop
The drop portion of the code implements the functionality of finding a drop zone that is optimal for dropping an object of given dimensions. It contains three main functionalities: 1) handle drop request: determines if and where to drop an item into the LFB; 2) handle post drop request: generates/uploads photos and videos and sends the maxZ and bag full percentage to the VLSG; 3) pre/post compare: checks for any errors with the drop such as item not in bag. 

#### Using Drop Manager
The drop manager takes in a container and uses that to process the requests that are received in JSON form. The following code sample illustrates how to use the drop manager and can be run in `offline_test.cpp`. This specifically demonstrates `handle_drop_request`. Others can be found in `offline_test`. 
```c++
int main(int argc, char** argv)
{
  // Loading a container from disk (the filepath will not work on your machine)
  std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session;
  std::shared_ptr<fulfil::depthcam::mocks::MockSession> mock_session_tray;
  std::string mock_serial = reader->Get(reader->get_default_section(), "mock_serial", "NOT USED");
  start_mock_session(directory_path, mock_session, mock_serial);
  if (mock_session == nullptr) Logger::Instance()->Fatal("The mock session was improperly initialized!!!");
  auto drop_live_viewer = make_shared<LiveViewer>(std::make_shared<std::string>(std::string("/home/steve/Documents/live_drop_image/bay0/")), LFB_config_reader);

  // Creating the DropManager
  std::shared_ptr<fulfil::dispense::drop::DropManager> manager = std::make_shared<fulfil::dispense::drop::DropManager>(mock_session_post, reader, LFB_config_reader, nullptr);
  
  // Creating the request
  shared_ptr<string> file_path = make_shared<string>();  //read json from file
  file_path->append(*directory_path);
  FileSystemUtil::join_append(*file_path, "json_request.json");
  std::ifstream ifs( *file_path);
  std::shared_ptr<nlohmann::json> request_json = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs));
  std::shared_ptr<std::string> request_id = std::make_shared<std::string>("000000000012");
  std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> offline_drop_details =
      std::make_shared<fulfil::dispense::commands::DropTargetDetails>(request_json, request_id); // 0

  // Processing the request
  manager->handle_drop_request(request_json, offline_drop_details, directory_path, nullptr, false); // parameters

  return 0;
}
```

### Dispense
The dispense section of the code is the code that handles all of the commands and tells which resources to do what when commands are received. It initializes the networking layer, it is the delegate for the networking layer, it initializes the drop manager, it is the delegate for the drop manager, and it also manages the processing queue for requests. It does a lot of stuff but most of it is just orchestrating a lot of functionality and not actually doing that functionality. It manages the delegation pipelines.

#### Image Persistence
The image persistence section of the code manages both what files need to be saved when executing a command as well as defines some interfaces to determine the name of files. The interfaces for determining the name of the files are primarily done in order to make testing easier.

##### Image Persistence Interfaces
The main interfaces used are the DispenseFileManager which is in charge of naming files. This relies on the TimeStamper interface which outlines a function to return the current timestamp (whenever you have timestamps testing is annoying so this allows easy mocking of timestamp functionality). DispenseImagePersistenceManager uses the DispenseFileManager to get the filename of the file and then handles the actual saving of file for a specific command.

There are also headers for the implementation of these classes in the code which have accompanying cpp files.

#### Using Dispense Manager
Creating the dispense manager is pretty straightforward, most of the heavy lifting of the initialization is done under the hood in the constructor. All you have to pass it are a bay number (typically 0 or 1), a LFB depthcam session, a tray depthcamera session, various config file readers, and a mongo connection. This is done under the hood in `bay_manager.h` which is invoked when the `BayManager` constructor is called in `main`. 
```c++
 ...
 try {
     std::shared_ptr<RealsenseManager> sensor_manager = std::make_shared<RealsenseManager>(
             std::make_shared<DeviceManager>());
     // pass in the connection here so that it can be used to retrieve configurations in dispense manager
     std::shared_ptr<RealsenseRunnerFactory> factory = std::make_shared<RealsenseRunnerFactory>(reader, conn);
     std::shared_ptr<RealsenseBayParser> parser = std::make_shared<RealsenseBayParser>(reader);
     BayManager<std::shared_ptr<Session>> manager(sensor_manager, factory, parser, expected_number_bays,
                                                  both_cameras_required);
     logger->Info("Start bay manager...");
     manager.start();
   } catch (const rs2::error & e) {
   ...
```

### Visualization
The code saves pertinent photos and videos at various stages of the dispense process. Some examples include: the tray before and after dispense, the LFB before and after dispense, as well as videos of the tray while an item is being dispensed. These photos and videos are piped to two visualization tools: ABIS and Live DepthCam Images. Both are a part of the Factory Data Tool (FDT). These visualization are uploaded from the local `Videos` folder to Google Cloud and an FTP server.

The saving of these photos and videos are handled by a `live_viewer` class. The actual upload lives in Fulfil.Depthcams and is called in `dispense_manager`.  

#### ABIS
Stands for Automated Bag Inspection System that has since evolved to include trays as well. It shows photos of the tray and LFB before and after each dispense, as well as a video of the dispense process from both the tray and LFB camera. This tool also enables manual verification to indicate if the correct type and number of product was dispensed. 

#### Live DepthCam Images
Includes solely images from the LFB camera, and represents the "decision making" process of the dispense software. It includes:
* Info: Whether drop target was successful or not. If successful, what percentage of bag is full and if item landed in bag. If unsuccessful, the type of error (ex/ bag full, damage risk)
* Damage risk heat map
* RGB image showing which markers were detected.
* Depth image: RGB image with point cloud superimposed over the bag
* Binary image showing location of the bag
* RGB image showing drop target
* RGB images showing the bag before and after the dispense
* Item detection showing where dispense item landed

Note: These images are only displayed for a short period of time after the dispense happens, and will be immediately overwritten once another dispense occurs on the same VLS. 

## Running Dispense Firmware
To run the firmware for dispense with the depth cam, you need to run the executable named `main`. This executable will kick of the process of searching for connections where it will listen for requests. 

### Live Testing Dispense
In addition to the unit tests in this library, it will likely be useful to actually test sending commands and receiving responses from the firmware to ensure that things are working as they are supposed to. The way to do this is to use the executable named `dispense_test_client`. The first command line argument to that executable should be the json payload that is being sent. It can technically be any payload you want and it will be encoded and sent. If the firmware can't handle it, it will send an error response.
