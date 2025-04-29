#include <iostream>
#include <chrono>
#include <ctime>

class Logger
{
public:
    static void info(const std::string &message)
    {
        std::cout << "[INFO] " << currentDateTime() << " " << message << std::endl;
    }

    static void error(const std::string &message)
    {
        std::cerr << "[ERROR] " << currentDateTime() << " " << message << std::endl;
    }

private:
    static std::string currentDateTime()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        return std::string(std::ctime(&now_time)).substr(0, 24); // remove newline
    }
};
