#include "logger_error.hpp"
#include <string>

namespace
{

struct TsLoggerErrorCategory : public std::error_category
{
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

const char* TsLoggerErrorCategory::name() const noexcept
{ return "tslogger"; }

std::string TsLoggerErrorCategory::message(int ev) const
{
    switch((TsLoggerStatus)ev)
    {
        case TsLoggerStatus::TS_LOGGER_OK:
            return "Success";
        case TsLoggerStatus::TS_LOGGER_ERR_SINGLE_INSTANCE:
            return "The single-instance object already exists";
        case TsLoggerStatus::TS_LOGGER_ERR_NOT_DIRECTORY:
            return "This should be a directory";
    }
    return "Unknown error";
}

const TsLoggerErrorCategory theTsLoggerErrorCategory {};

} // namespace

namespace tslogger
{

std::error_code make_error_code (TsLoggerStatus e)
{
    return {static_cast<int>(e), theTsLoggerErrorCategory};
}

std::error_code make_system_error (int e)
{
    return {e, std::generic_category()};
}

} // namespace tslogger
