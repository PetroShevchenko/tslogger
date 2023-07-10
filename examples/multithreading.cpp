#include "logger.hpp"
#include <atomic>
#include <signal.h>

using namespace tslogger;

static std::atomic<bool> g_terminate = false;

static void signal_handler(int signo)
{
    if (signo == SIGINT)
    {
        g_terminate = true;
    }
}

int main()
{
	std::error_code ec;
	const char *root = "/home/petro/1/2/3/"; 

	if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        std::cerr << "Unable to set SIGINT handler\n";
        exit(1);
    }

	Handler logHandler(root, DEBUG, ec);
	if (ec.value()) {
		std::cerr << "ERROR:(" << ec.value() << ") "<< ec.message() << "\n";
		exit(1);
	}

	std::cout << "All logging files will be stored into the " << root << " directory\n";

    std::thread loggerThread1([&](){
        Logger logger(logHandler.getQueuePtr(), nullptr);
        #define LOG_D(...) LOG(logger, DEBUG, __VA_ARGS__) 
        int a = 0xFF09;
        LOG_D("> %s:%d | Thread1 | This is a hex variable value: %x\n",  __func__, __LINE__, a);
    });

    std::thread loggerThread2([&](){
    	std::string filename;
    	add_timestamp_prefix("_thread2.log", filename);
        Logger logger(logHandler.getQueuePtr(), filename.c_str());
        const char *text = "This is a \t\ttest error message from the Thread2\n";
        LOG(logger, ERROR, "%s", text);
    });

    root = "/home/petro/1/2/3/4/";
    logHandler.root(root, ec);
	if (ec.value()) {
		std::cerr << "ERROR:(" << ec.value() << ") "<< ec.message() << "\n";
		exit(1);
	}
	std::cout << "Root directory has been changed to the " << root << " directory\n";

    std::thread loggerThread3([&](){
        Logger logger(logHandler.getQueuePtr(), "test.log");
        const char *text = "This is a test message from the Thread3\n";
        logger.log(INFO, "%s", text);
    });

    std::thread loggerThread4([&](){
        Logger logger(logHandler.getQueuePtr(), "test.log");
        const char *text = "This is a test message from the Thread4\n";
        logger.log(WARNING, "%s", text);
    });

    std::thread handlerThread([&](){
      	while(!g_terminate)
    	{
			logHandler.process();
    	}
    });

    std::cout << "Press Ctrl+C to stop logging and exit\n";

 	if (loggerThread1.joinable())
    	loggerThread1.join();

 	if (loggerThread2.joinable())
	    loggerThread2.join();

 	if (loggerThread3.joinable())
    	loggerThread3.join();

 	if (loggerThread4.joinable())
	    loggerThread4.join();

 	if (handlerThread.joinable())
	    handlerThread.join();

    exit(0);
}