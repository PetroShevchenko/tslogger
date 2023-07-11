#include <logger.hpp>
#include <thread>
#include <vector>
#include <chrono>

using namespace tslogger;

int main()
{
    std::error_code ec;
    const char *root = "/home/petro/logs/";

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
