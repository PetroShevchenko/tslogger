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
    const char *root = "/home/petro/logs/1/2/3/";

    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        std::cerr << "Unable to set SIGINT handler\n";
        exit(1);
    }

    Handler logHandler(root, DEBUG, std::clog, ec);
    if (ec.value()) {
        std::cerr << "ERROR:(" << ec.value() << ") "<< ec.message() << "\n";
        exit(1);
    }

    std::cout << "All logging files will be stored into the " << root << " directory\n";

    std::thread loggerThread1([&](){
        Logger logger(
                    logHandler.get_queue_ptr(),
                    nullptr,
                    FLAGS_OUTPUT_TO_ALL,
                    LINE_FORMAT_MSG_ONLY
                );

        logger.default_level(DEBUG);

        std::vector<int> v;
        for (int i = 0; i < 32; ++i)
            v.push_back(i);
        logger << "Vector \"std::vector\" of integers: " << v;

        std::array<int, 32> a;
        for (int i = 0; i < 32; ++i)
            a[i] = i;
        logger << "Array \"std::array\" of integers: " << a;
    });


    std::thread loggerThread2([&](){
        Logger logger(
                    logHandler.get_queue_ptr(),
                    nullptr,
                    FLAGS_OUTPUT_TO_ALL,
                    LINE_FORMAT_MSG_ONLY
                );

        logger.default_level(DEBUG);

        std::vector<char> v;
        for (char i = 'a'; i <= 'z'; ++i)
            v.push_back(i);
        logger << "Vector \"std::vector\" of characters: " << v;

        std::array<char, 26> a;
        char i = 'a';
        for (int j = 0; j < 26; ++j)
            a[j] = i++;
        logger << "Array \"std::array\" of characters: " << a;        
    });

    std::thread loggerThread3([&](){
        Logger logger(
                    logHandler.get_queue_ptr(),
                    nullptr,
                    FLAGS_OUTPUT_TO_ALL,
                    LINE_FORMAT_MSG_ONLY
                );

        logger.default_level(DEBUG);

        const char a[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
         'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\n', '\0'} ;
        logger << "Array \"const char []\" of characters: " << a;
        char b[] = "ABCD\n";
        logger << "Array \"char []\" of characters: " << b;

        int c[32];
        for (int i = 0; i < 32; ++i)
            c[i] = i;
        logger << "Array \"int []\" of integers: " << c;
    });

    std::thread handlerThread([&](){
        while(!g_terminate)
        {
            logHandler.process();
        }
    });

    std::cout << "-----------------------------------------\n";
    std::cout << "| Press Ctrl+C to stop logging and exit |\n";
    std::cout << "-----------------------------------------\n";

    if (loggerThread1.joinable())
        loggerThread1.join();

    if (loggerThread2.joinable())
        loggerThread2.join();

    if (loggerThread3.joinable())
        loggerThread3.join();

    if (handlerThread.joinable())
        handlerThread.join();

    exit(0);
}