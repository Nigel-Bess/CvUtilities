# Fulfil.CPPHelpers
The purpose of this library is to gather common use utilities that can be helpful for different C++ projects at Fulfil. Code that is added to this library should not be specific to one project, but code that would be useful to many projects. For example: code that would be beneficial to include in this repo would be generic networking code that abstracts away some of the complexity of networking. An example of code that should not go in this repository is code that parses requests for depth camera queries.

## Dependencies
This project requires only dependencies related to specific utilities. For example: if you want to use the eigen utilities, you will need to have eigen installed.
### Eigen Util Dependencies
* Eigen 3.3.9
### Processing Queue Dependencies 
* Requires thread which should be part of the standard library.

## Integration
This project follows the new project structure that is outlined in the [C++ Styleguide](https://github.com/Fulfil0518/styleguides/blob/master/cpp-styleguide.md). To integrate it, just add it as a submodule, add the line `add_subdirectory(path_to_this_library)` in your main cmake file, and add `find_library(Fulfil.CPPHelpers)` to your cmake file and include the library in your target.

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

## Outline
* [Eigen Utils](#eigen-utils)
    * [Building Dynamically Sized Matrices](#building-dynamically-sized-matrices)
    * [Filtering Matrices](#filtering-matrices)
* [General Utilities](#general-utils)
    * [Depth Pixel](#depth-pixel)
    * [Pixel](#pixel)
    * [Point 3D](#point3d)
    * [File System Util](#filesystemutil)
        * [Working with Files](#working-with-files)
        * [Working with Directories](#working-with-directories)
* [Processing Queue](#processing-queue)
    * [Basic Processing](#basic-processing)
    * [Purging Queue](#purging-a-processing-queue)
    * [Processing With Multiple Threads](#processing-with-multiple-threads)
* [Networking](#networking)
    * [Socket Information](#socket-information)
    * [Socket Manager](#socket-manager)
    * [Socket Command Header](#socket-command-header)
    * [Socket Response](#socket-response)
    * [Socket Command Parser](#socket-command-parser)
    * [Socket Network Manager and Delegate](#socketnetworkmanager-and-delegate)
        * [General Flow of Commands](#general-flow-of-receiving-commands-and-sending-responses)
    

## Eigen Utils

### Building Dynamically Sized Matrices
One common task with eigen is building matrices of variable length, especially if you are conditionally adding columns from another matrix. You may not know what the final size of the matrix will be, and there is some verbose code to manage this. The Matrix3Xd builder simplifies the code you have to write in order to create a dynamically sized matrix. The following code example shows how this works.
```c++
std::shared_ptr<fulfil::utils::eigen::Matrix3XdBuilder> builder = std::make_shared<fulfil::utils::eigen::Matrix3XdBuilder>();
//Should print out an empty matrix
std::cout << *builder->get_matrix() << std::endl;
for(int i = 0; i < 10; i ++)
{
  builder->add_row(i, i + 10, i + 20);
}
//Should print out a matrix (0,10,20)...(9,19,29)
std::cout << *builder->get_matrix() << std::endl;
```

### Filtering Matrices
A common task when working with point clouds (which are often represented withe eigen matrices) is to filter out points that are not important or relevant. The following code example illustrates how to do that with Matrix3XdFilter and Matrix3XdPredicate.
```c++
//Creating the initial matrix
std::shared_ptr<fulfil::utils::eigen::Matrix3XdBuilder> builder = std::make_shared<fulfil::utils::eigen::Matrix3XdBuilder>();
for(int i = 0; i < 5; i ++)
{
  builder->add_row(-i, -(i + 10), -(i + 20));
}
for(int i = 0; i < 5; i++)
{
  builder->add_row(i, i + 10, i + 20);
}
//Creating a predicate using the CustomMatrix3dPredicate class to keep just points with all negative values.
std::shared_ptr<fulfil::utils::eigen::Matrix3dPredicate> negative_only_predicate =
    std::make_shared<fulfil::utils::eigen::CustomMatrix3dPredicate>(
        [](const fulfil::utils::eigen::Matrix3dPoint& point)
        {
          return point(0) < 0 && point(1) < 0 && point(2) < 0;
        });
//Using the previously created predicate and the CustomMatrix3dPredicate to keep points that don't have all negative values.
std::shared_ptr<fulfil::utils::eigen::Matrix3dPredicate> positive_or_zero_predicate =
    std::make_shared<fulfil::utils::eigen::CustomMatrix3dPredicate>(
        [negative_only_predicate](const fulfil::utils::eigen::Matrix3dPoint& point)
        {
          return !(negative_only_predicate->evaluate(point));
        });
//Creating filters based on the previously created predicates.
std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> negative_only_filter = std::make_shared<fulfil::utils::eigen::Matrix3XdFilter>(negative_only_predicate);
std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> positive_or_zero_filter = std::make_shared<fulfil::utils::eigen::Matrix3XdFilter>(positive_or_zero_predicate);

//Should return a matrix of (-1,-11,-21)...(-4,-14,-24)
std::cout << *negative_only_filter->filter(builder->get_matrix()) << std::endl;
//Should return a matrix of (0,-1,-10)...(4,14,24)
std::cout << *positive_or_zero_filter->filter(builder->get_matrix()) << std::endl;
```

## General Utils

## Depth Pixel
The depth pixel class is a simple class that is designed to contain coordinate for a pixel as well as the depth value at that pixel. The motivation for this was to reduce verbosity when combining depth data with pixel coordinates. Previosuly (and in some parts of other code bases still) this data was stored as a pair of a pixel and a double for depth data.

## Pixel
The pixel class is a simple class that stores the x and y coordinates of a pixel. The motivation for this was to allow for the concept of a pixel to be used without having to include opencv headers.

## Point3D
Point3D is a convenience structure designed to keep track of 3D points. The motivation for this came from code working with Eigen matrices representing points being verbose and not straightforward. It mostly serves to add clarity to working with 3D coordinates.

## FileSystemUtil
Filesystem util provides a variety of common use functions that encapsulate working with files into a simpler interface. The goal with this was to provide an API to use the filesystem that mimics the simplicity of python.

### Working with Files
There is functionality to write to files in multiple formats, read from files in multiple formats, and interact with files. The following code samples illustrate them.
```c++
//Reading and Writing any type of data.
double data[2] = {1.0, 2.0};
const char* writeable_data = (const char*) data;
fulfil::utils::FileSystemUtil::write_to_file(writeable_data, 2 * sizeof(double), "tmp");

std::shared_ptr<std::string> read_data = fulfil::utils::FileSystemUtil::get_string_from_file("tmp");
double read_data_values[2];
std::memcpy(&read_data_values, read_data->c_str(), 2 * sizeof(double));

std::cout << read_data_values[0] << std::endl;
std::cout << read_data_values[1] << std::endl;

// Doing the write only if the file doesn't exist
double data[2] = {1.0, 2.0};
const char* writeable_data = (const char*) data;
fulfil::utils::FileSystemUtil::write_to_file_if_not_exists(writeable_data, 2 * sizeof(double), "tmp");

std::shared_ptr<std::string> read_data = fulfil::utils::FileSystemUtil::get_string_from_file("tmp");
double read_data_values[2];
std::memcpy(&read_data_values, read_data->c_str(), 2 * sizeof(double));

std::cout << read_data_values[0] << std::endl;
std::cout << read_data_values[1] << std::endl;
```
```c++
//Writing a vector of booleans to a file.
std::shared_ptr<std::vector<bool>> flags = std::make_shared<std::vector<bool>>();
flags->push_back(true);
flags->push_back(false);
flags->push_back(true);
flags->push_back(false);
fulfil::utils::FileSystemUtil::write_flags_to_file(flags, std::make_shared<std::string>("tmp"));

std::shared_ptr<std::vector<bool>> read_flags = fulfil::utils::FileSystemUtil::read_flags_from_file(std::make_shared<std::string>("tmp"));
for(bool flag : *flags)
{
  std::cout << flag << std::endl;
}
```
```c++
//Check if a file exists
bool file_exists = fulfil::utils::FileSystemUtil::file_exists("tmp");
std::cout << file_exists << std::endl;
```

### Working with Directories
There are several functions for interacting with directories. The following code examples illustrate them.
```c++
//Creating directories and checking if they exist
bool directory_exists = fulfil::utils::FileSystemUtil::directory_exists("tmp");
std::cout << directory_exists << std::endl;

//Will throw an error if the directory exists.
fulfil::utils::FileSystemUtil::create_directory("tmp");
directory_exists = fulfil::utils::FileSystemUtil::directory_exists("tmp");
std::cout << directory_exists << std::endl;

//Will not create a directory because it already exists.
fulfil::utils::FileSystemUtil::create_dir_if_not_exist("tmp");
```
```c++
//Getting Contents of Directory
//Will throw an error if the directory does not exist.
std::shared_ptr<std::vector<std::shared_ptr<std::string>>> contents = fulfil::utils::FileSystemUtil::get_files_in_directory(std::make_shared<std::string>("."));
for(std::shared_ptr<std::string> content_name : *contents)
{
  std::cout << *content_name << std::endl;
}
```

## Processing Queues
The purpose of the processing queue util is to have a generic queue that processes a queue of requests asynchronously. There is the main queue class which handles all of the asynchronous code. All that needs to be implemented is a delegate that determines how to process a request and how to communicate a response. Both the processing of the request and the sending of the response are run asynchronously.

### Basic Processing
The basics of the processing queue is that it has a queue of requests to execute and will continuosuly loop through the queue until explicitly told otherwise. It does not stop processing when there are no items in the queue. The following code shows a simple example.
```c++
// Processing requests asynchronously and sending responses asynchronously

// First define a class that implements the delegate with the desired types
// It needs to inherit from std::enable_shared_from_this to be used as a weak pointer.
// This example is simple and doesn't illustrate that both the process_request function can be asynchronous calls to some other object.
class SpyProcessingQueueDelegate : public ProcessingQueueDelegate<int, int>, public std::enable_shared_from_this<SpyProcessingQueueDelegate>
{
 public:
  std::unique_ptr<std::stack<int>> responses;
  SpyProcessingQueueDelegate()
  {
    this->responses = std::make_unique<std::stack<int>>();
  }
  void send_response(int response)
  {
    this->responses->push(response);
  }
  int process_request(int request)
  {
    return request;
  }
};

std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
queue->delegate = delegate->weak_from_this();
queue->push(1, 0);
queue->push(2, 0);
queue->push(3, 0);
std::thread t([queue](){
  queue->start_processing(0);
});
t.detach();
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
queue->stop_processing();
```

### Purging a Processing Queue
An additional feature the processing queue contains is one that will loop through the inner queue in linear time and remove all requests that don't pass a provided predicate. This is thread safe so it can be executed while the queue is processing.
```c++
// Defining an example delegate
class SpyProcessingQueueDelegate : public ProcessingQueueDelegate<int, int>, public std::enable_shared_from_this<SpyProcessingQueueDelegate>
{
 public:
  std::unique_ptr<std::stack<int>> responses;
  SpyProcessingQueueDelegate()
  {
    this->responses = std::make_unique<std::stack<int>>();
  }
  void send_response(int response)
  {
    this->responses->push(response);
  }
  int process_request(int request)
  {
    return request;
  }
};

// Defining an example predicate that takes in a function pointer to be used as the predicate.
class SpyProcessingQueuePredicate : public ProcessingQueuePredicate<int>
{
 private:
  bool (*predicate)(int);
 public:
  explicit SpyProcessingQueuePredicate(bool (*predicate)(int))
  {
    this->predicate = predicate;
  }

  bool should_keep(int i) override
  {
    return this->predicate(i);
  }
};

std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
queue->delegate = delegate->weak_from_this();
queue->push(1, 0);
queue->push(2, 0);
queue->push(1, 0);

std::shared_ptr<SpyProcessingQueuePredicate> predicate = std::make_shared<SpyProcessingQueuePredicate>([](int i){return i != 1;});

std::thread t([queue](){
  queue->start_processing(0);
  //use the predicate to purge the queue
  queue->purge_queue(predicate);
});
t.detach();
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
queue->stop_processing();
```

### Processing with Multiple Threads
The way that the processing queue is designed allows for the `start_processing` function to be called multiple times, each time on a different thread, to use more threads to process the queue.

```c++
std::shared_ptr<SpyProcessingQueueDelegate> delegate = std::make_shared<SpyProcessingQueueDelegate>();
std::shared_ptr<ProcessingQueue<int, int>> queue = std::make_shared<ProcessingQueue<int, int>>();
queue->delegate = delegate->weak_from_this();
for(int i = 0; i < 10; i++)
{
  queue->push(i, 0);
}

std::thread t([queue]()
{
  queue->start_processing(500);
});
t.detach();
std::thread t2([queue]()
{
  queue->start_processing(0);
});
t2.detach();
std::this_thread::sleep_for(std::chrono::milliseconds(1000));
queue->stop_processing();
```

## Networking

### Socket Networking

#### Socket Information
The socket information object contains some basic information about the socket that is being opened. Right now this only contains the port number but attributes will likely be added when client side socket networking code is written. It is still wrapped into this object for flexibility in the future.

#### Socket Manager
The socket manager manages the process of creating and connecting to sockets. Right now a limitation is that the socket manager can only connect to one client at a time. The following code illustrates how to create a socket and start searching for a connection:
```c++
// Initializing the socket information for the socket.
std::shared_ptr<fulfil::utils::networking::SocketInformation> socket_info = std::make_shared<fulfil::utils::networking::SocketInformation>(port);
// Initializing the socket manager with the socket information created.
std::shared_ptr<fulfil::utils::networking::SocketManager> manager = std::make_shared<fulfil::utils::networking::SocketManager>(socket_info);
// Creating the socket
manager->create_socket();
// Initiating a search for a connection (this will hang until a connection is established)
manager->connect_socket();
```

#### Socket Command Header
The purpose of teh socket command header is to define a format for socket communication. The Socket Command Header contains a 12 byte id and a 16 bit integer representing the number of bytes in the payload of the command. This is then used to pass the payload to a parser that will make sense of the command in the context of the project.

The network manager handles most of this but the following code shows how it reads from a socket using the socket command header:
```c++
// Initializes a header object and sets the data to all zeros.
std::shared_ptr<fulfil::utils::networking::SocketCommandHeader> header = std::make_shared<fulfil::utils::networking::SocketCommandHeader>();
// Sets all of the bytes in the header to zero to reduce errors from malformed requests.
std::memset(&(*header), 0, sizeof(fulfil::utils::networking::SocketCommandHeader));
// Attempts to read data equal to the size of the header from the socket.
int bytes_read = read(this->socket_manager->connection_fd, &(*header), sizeof(fulfil::utils::networking::SocketCommandHeader));
// If the socket is disconnected it will not read any bytes.
if (bytes_read <= 0)
{
  throw fulfil::utils::networking::errors::SocketDisconnectionException();
}
// If it reads less than the full size of the header, the request was invalid.
if(bytes_read < sizeof(fulfil::utils::networking::SocketCommandHeader))
{
  throw fulfil::utils::networking::errors::SocketHeaderParsingException();
}
//You now have a valid header
std::cout "header_id: " << header->command_id << " bytes in payload: " << header->bytesleft << std::endl;
```

#### Socket Response
The socket response defines an interface for getting information and size of both the header and payload of a response. This allows there to be complete flexibility in format of responses that are sent. The functions that return void* should return pointers that can be freed by calling free. The following code illustrates how the header interface is used in the Socket Network Manager:
```c++
void* header = response->header();
int bytes_sent = send(this->socket_manager->connection_fd, header, response->header_size(), 0);
free(header);
if(bytes_sent == -1)
{
  // The header failed to send
}
else
{
  void* payload = response->payload();
  bytes_sent = send(this->socket_manager->connection_fd, payload, response->payload_size(), 0);
  free(payload);
  if (bytes_sent == -1)
  {
    // The payload failed to send.
  }
}
```
#### Socket Command Parser
The purpose of the socket command parser is to allow for different type of requests to be parsed while taking into consideration that the size has to be known in order to read from the socket in the first place. The parser will be provided the id from the Socket Command Header and the payload from the request in the form of a string. It provides the interface to go from the info from the command header to a new type of request. The following code example shows how that could be done:
```c++
// Defining a parser
class SpyCommandParser : public fulfil::utils::networking::SocketCommandParser<int>
{
  int parse_payload(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> request_id) override
  {
    char* buffer = new char(sizeof(int));
    std::memcpy(buffer, payload->c_str(), sizeof(int));
    int request = (int) *buffer;
    return request;
  }
};

// The socket manager takes in a parser and does the following with the socket

// Initializes a header object and sets the data to all zeros.
std::shared_ptr<fulfil::utils::networking::SocketCommandHeader> header = std::make_shared<fulfil::utils::networking::SocketCommandHeader>();
std::memset(&(*header), 0, sizeof(fulfil::utils::networking::SocketCommandHeader));
// Attempts to read data equal to the size of the header from the socket.
int bytes_read = read(this->socket_manager->connection_fd, &(*header), sizeof(fulfil::utils::networking::SocketCommandHeader));
if (bytes_read <= 0)
{
throw fulfil::utils::networking::errors::SocketDisconnectionException();
}
if(bytes_read < 14)
{
  throw fulfil::utils::networking::errors::SocketHeaderParsingException();
}

// Now that it has the header, it reads the payload from the socket.
// Checks for invalid bytes left
if(header->bytesleft < 0)
{
  throw fulfil::utils::networking::errors::InvalidSocketPayloadSizeException(std::make_shared<std::string>(header->command_id, 12));
}
// Creates a buffer for the payload
char* data = new char[header->bytesleft];
memset(data, 0, header->bytesleft);
// Reads payload sized data from the socket.
int bytes_read = read(this->socket_manager->connection_fd, data, header->bytesleft);
// Checks if data read = expected data read
if (bytes_read != header->bytesleft)
{
  throw fulfil::utils::networking::errors::UnexpectedSocketReadSizeException(
    std::make_shared<std::string>(header->command_id, 12),
    bytes_read, header->bytesleft);
}

// It then uses the parser to get the request from the payload
try
{
  // Tries to parse the request.
  Request request = this->command_parser->parse_payload(payload, command_id);
}
catch (const fulfil::utils::networking::errors::InvalidCommandFormatException& command_format_exception)
{
  // Throws an error when the parsing fails
  this->handle_error(command_format_exception);
} 
```

#### SocketNetworkManager and Delegate
The socket network manager is designed to handle processing commands that come in, delegating the parsing and processing of them and send responses. The idea behind it is to encapsulate all of the communication aspects and allow the user of the library to just provide functionality of how to parse a command, how to process a command, and how to encode the response. The following code example shows how to use the network manager and delegate:
```c++
// Defining the parser
class SpyCommandParser : public fulfil::utils::networking::SocketCommandParser<int>
{
  int parse_payload(std::shared_ptr<std::string> payload, std::shared_ptr<std::string> request_id) override
  {
    char* buffer = new char(sizeof(int));
    std::memcpy(buffer, payload->c_str(), sizeof(int));
    int request = (int) *buffer;
    return request;
  }
};
// Defining the delegate
class SpyDelegate : public fulfil::utils::networking::SocketNetworkManagerDelegate<int>,
    public std::enable_shared_from_this<fulfil::utils::networking::SocketNetworkManagerDelegate<int>>
{
  void did_receive_request(int request) override
  {
    std::cout << "received valid request " << request << std::endl;
  }
  void did_receive_invalid_request(std::shared_ptr<std::string> request_id) override
  {
    std::cout << "received invalid request with id: " << *request_id << std::endl;
  }
  void did_receive_invalid_request() override
  {
    std::cout << "received invalid request without id" << std::endl;
  }
};

// Defining the response
class SimpleResponse : public fulfil::utils::networking::SocketResponse
{
   int header_size() override
   {
      return sizeof(fulfil::utils::networking::SocketCommandHeader);
   }
   void* header() override
   {
      fulfil::utils::networking::SocketCommandHeader* header = new fulfil::utils::networking::SocketCommandHeader;
      std::memcpy(&header->command_id, "123456789012", 12);
      header->bytesleft = std::strlen("sample response");
      return header;
   }
   int payload_size() override
   {
      return std::strlen("sample response");
   }
   void* payload() override
   {
      char* payload = new char(std::strlen("sample response"));
      std::memcpy(payload,  "sample response", std::strlen("sample response"));
      return payload;
   }
};

// Code that will actually create a socket, search for a connection, receive a command, and send a response.

// Setting up socket manager
std::shared_ptr<fulfil::utils::networking::SocketInformation> socket_info = std::make_shared<fulfil::utils::networking::SocketInformation>(port);
std::shared_ptr<fulfil::utils::networking::SocketManager> manager = std::make_shared<fulfil::utils::networking::SocketManager>(socket_info);

// Instantiating the parser
std::shared_ptr<fulfil::utils::networking::SocketCommandParser<int>> parser = std::make_shared<SpyCommandParser>();

// Instantiating the Delegate
std::shared_ptr<SpyDelegate> delegate = std::make_shared<SpyDelegate>();

// Using the parser and socket manager to instantiate the network manager.
std::shared_ptr<fulfil::utils::networking::SocketNetworkManager<int>> network_manager =
    std::make_shared<fulfil::utils::networking::SocketNetworkManager<int>>(manager, parser);
// Assigning the delegate.
network_manager->delegate = delegate->weak_from_this();

// Starts listening and waits 10000 milliseconds before sending a response.
network_manager->start_listening();
std::this_thread::sleep_for(std::chrono::milliseconds(10000));

// Sends a response
network_manager->send_response(std::make_shared<SimpleResponse>());

// Stops listening on the socket.
network_manager->stop_listening();
```
##### General Flow of Receiving Commands and Sending Responses
1. Command header received by network manager
1. Parser is called to parse the contents of the payload
1. Delegate is called to process the parsed request (if parsing was successful)
1. It is up to the delegate to do anything with the request, if they want to send a response back, they need to call the send_response function on the SocketNetworkingManager.


## Logging 

### Overview
Logging module instantiates a single logger for the project. The logger will report to both a file and to the console. The file logger will create a new file to report to each day the application is run in the logging directory and append the date created to the file suffix. By default the logger places the logging directory in `${CMAKE_SOURCE_DIR}/logs` (`${CMAKE_SOURCE_DIR}` will be in the outermost project directory). The loggers are ensured thread safe by calling mutex locks, so use judiciously in concurrent hot code paths. Each log message is prefixed with 
```
[{clock time} {time difference from utc}] [elapsed {time from last logging call in nano sec}] [-{log message level}-] [thread {thread id}]

[18:14:36 -07:00] [elapsed 19us] [-error-] [thread 680]
```

### Get Instance
The first time a logging instance is called, the logger is created. The logger directory and file prefix cannot be changed after the first initialization. Each successsive time that `Instance()` is called, a pointer to the logger object will be returned. Note that logging calls made in libs will be reported in the same location as those made in the main project (i.e. log calls made in CPPUtils functions will report to the same file as the logging calls made in the Fulfil.Dispense code). 
```
#include <Fulfil.CPPUtils/logging.h>

Logger* logger = Logger::Instance(); // will return default

Logger* Instance(std::string log_dir="cmake_source_dir/logs", std::string file_prefix = "daily_logger",
                                    Level file_level = Level::Debug, Level console_level = Level::Debug);

```
### Change Log Levels
The reporting level for the console logger and the file logger can be changed throughout the logger's lifetime independently of one another. In order of severity logging message levels are **{Trace, Debug, Info, Warn, Error, Fatal, TurnOff}**. Trace should never be the set log level in production. Info should almost always be reported in production unless there are serious performance concerns. If the logger needs to be deactivated, set level to TurnOff.
```
Logger::Instance()->SetFileLogLevel(Logger::Level::Trace);
Logger::Instance()->SetConsoleLogLevel(Logger::Level::TurnOff);

```
### Logger specific exception
To throw a logger error specifically (to report and issue with the logger itself).
`Logger::Instance()->ThrowException(std::string msg)`

### Log message
The logger can take a variable number of arguments and accepts format strings similar to those used by python's `str.format()` function. The first argument should always be a string type if you intend to us the formatting function. However you can log other types so long as they are the only message argument. See fmt library for more detail about format string capabilities (https://github.com/fmtlib/fmt).
```
Logger::Instance()->Info("You can use {} format strings and input more than {} argument.", "pythonic", 1);
// reports -> You can use pythonic format strings and input more than 1 argument.

Logger::Instance()->Debug("Even decimal formatting is accpeted: {:.2f}", 3.266859073)
// reports -> Even decimal formatting is accpeted: 3.27

Logger::Instance()->Error(5); // reports -> 5

try{...}
catch(std::exception &e){
   Logger::Instance()->Fatal("You can add custom messages to disambiguated thrown error messages\n{}", e.what());
}
```
