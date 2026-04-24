#ifndef _TS_LOGGER_PLATFORM_HPP
#define _TS_LOGGER_PLATFORM_HPP

#include <ctime>
#include <string>
#include <system_error>
#include <thread>

namespace tslogger::platform
{

bool create_directories(const std::string &path, std::error_code &ec);
bool is_directory(const std::string &path, std::error_code &ec);
bool append_to_file(const std::string &path, const std::string &text, std::error_code &ec);
bool localtime_safe(std::time_t ts, std::tm &out);
std::string thread_id_to_string(std::thread::id id);

} // namespace tslogger::platform

#endif // _TS_LOGGER_PLATFORM_HPP
