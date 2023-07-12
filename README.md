# tslogger
tslogger stands for Thread Safe Logger. The logger is written in C++17

## Main features

* The logger works in separate threads
* The log function parameters format is the similar to the printf function
* The << operator is overloaded to output simple types and STL containers like std::vector
* The logger instances use std::shared_ptr to the message queue
* The thread safe message queue is created inside the log handler
* The log handler is an single-instance object. There is only one log handler in the application
* A new logger should be created for each thread, from which an user wants to output logs
* All log files are stored into the root directory
* The root directory is created while the log handler object is constructing
* There is a max logging level to print out only messages, which log level is less than or equal the max logging level
* The root directory and the max logging level can be changed on the fly
* The log file name can be the same or different for any threads
* The log file name is set each time when the message is logged
* Logging can also be done to a stream (clog, cout, cerr etc) at the same time as logging to a file in any combination of these options
* The output stream is set on the log handler side

## Logger diagram

~~~
 _______________          _______________          _______________
|    Logger     |        |    Logger     |        |    Logger     |
|    Thread 1   |        |    Thread 2   |        |    Thread N   |
|_______________|        |_______________|        |_______________|
        |                        |                        |
        |                 _______|______                  |
        |                |              |                 |
        |________________|  Safe Queue  |_________________|
                         |______________| 
                                 |
                          _______|______
                         |    Handler   |
                         |    Thread    |
                         |______________|
                            |         |
                      ______|_____   _|_________    
                     |    File    | |   Stream  |  
                     |____________| |___________|

Log message:
------------
1. log level
2. message string
3. thread id
4. log file name
5. output log format
6. flags
7. timestamp

Handler:
--------
1. root path to store logs
2. max logging level
3. receiving a message from the the message queue
4. storing a mesage to different log files
5. changing the root path and max logging level on the fly
~~~

## Build

*Note: The compilation has been tested only on Ubuntu Linux*

To build the project launch the following commands in the command prompt:
~~~
user@host:~/tslogger$ mkdir build && cd build
user@host:~/tslogger/build$ cmake ..
user@host:~/tslogger/build$ make
~~~

## Usage examples

To use <b>tslogger</b> in your own project, follow these steps:
1. Copy the <b>api/</b> folder with its content to your project
2. Copy <b>libtslogger.a</b> to the <b>lib/</b> subdirectory
3. Create your own source files in the <b>src/</b> subdirectory
4. Create your own <b>CMakeLists.txt</b> file

*Note: You can use any folder names instead of api/, src/, lib/*

Here is the directory tree of the new sample project:
~~~
myproject
├── api
│    ├── error.hpp
│    ├── logger.hpp
│    └── safe_queue.hpp
├── CMakeLists.txt
├── lib
│    └── libtslogger.a
└── src
    └── simple_example.cpp
~~~

CMakeList.txt:
~~~cmake
cmake_minimum_required (VERSION 3.5)

set(PROJECT_NAME "myproject")

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/api)
set(LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)

set(SRC_LIST ${SRC_DIR}/simple_example.cpp)

add_definitions(-DUSE_TS_LOGGER)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} tslogger)

target_link_directories(${PROJECT_NAME} PRIVATE ${LIB_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR})
~~~

simple_example.cpp:
```c++
#include <logger.hpp>
#include <thread>
#include <vector>
#include <chrono>

using namespace tslogger;

int main()
{
    std::error_code ec;
    const char *root = "/home/user/logs/";

    Handler logHandler(root, DEBUG, std::clog, ec);
    if (ec.value())
    {
        std::cerr << ec.message() << "\n";
        exit(1);
    }

    std::thread handlerThread([&](){
        while(true)
        {
            logHandler.process();
        }
    });

    handlerThread.detach();

    std::thread loggerThread([&](){
        Logger logger(
                logHandler.get_queue_ptr(),
                "simple_example.log",
                FLAGS_OUTPUT_TO_ALL,
                LINE_FORMAT_MSG_ONLY
            );

        logger << "Hello, tslogger!\n";

        std::vector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        logger << "vector: " << v;

        logger.format(LINE_FORMAT_ALL);

        unsigned int hexValue = 0xFF00A55A;

        logger.log(DEBUG, "Value in hex format: %x\n", hexValue);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    if (loggerThread.joinable())
        loggerThread.join();

    exit(0);
}
```

*Note: Replace the path in the line <b>const char \*root = \"/home/user/logs/\";</b> on your own*

Output:
~~~
Hello, tslogger!
vector: { 1, 2, 3 }
[DEBUG] 2023-07-12 11:06:26 thread_id: 0x7ffff6ffe640 Value in hex format: 0xff00a55a
~~~

The log file can be shown by the following command:
~~~
user@host:~$ cat /home/${USER}/logs/simple_example.log
~~~

## Licence

The tslogger library is distributed under Apache license version 2.0.
LICENSE file contains license terms.
