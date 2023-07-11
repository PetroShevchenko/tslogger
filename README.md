# tslogger
tslogger stands for Thread Safe Logger. The logger is written in C++17

## Main features

* The logger works in separate threads
* The log function parameters format is the similar to the printf function 
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

Note: The compilation has been tested only on Ubuntu Linux

To build the project launch the following command in the command prompt:
~~~
user@host:~/tslogger$ mkdir build && cd build
user@host:~/tslogger/build$ cmake ..
user@host:~/tslogger/build$ make
~~~

## Licence

The tslogger library is distributed under Apache license version 2.0.
LICENSE file contains license terms.
