#ifndef _TS_LOGGER_ERROR_HPP
#define _TS_LOGGER_ERROR_HPP
#include <system_error>

enum class TsLoggerStatus
{
    TS_LOGGER_OK = 0,
    TS_LOGGER_ERR_SINGLE_INSTANCE,
    TS_LOGGER_ERR_NOT_DIRECTORY,
};

namespace std
{
template<> struct is_error_condition_enum<TsLoggerStatus> : public true_type {};
}

namespace tslogger
{
std::error_code make_error_code (TsLoggerStatus e);
std::error_code make_system_error (int e);
}

#endif

