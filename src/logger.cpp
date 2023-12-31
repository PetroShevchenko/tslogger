#include <cstdarg>
#include <cstddef>
#include <vector>
#include <fstream>
#include "logger.hpp"

namespace tslogger
{

bool Handler::s_init = false;
std::mutex Handler::s_mutex;

time_t timestamp()
{
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void timestamp_to_date_time_string(time_t ts, std::string &out)
{
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime( &ts ));
    out.reserve(strlen(buf));
    out = buf;
}

void add_timestamp_prefix(const char *filename, std::string &out)
{
    time_t ts = timestamp();
    timestamp_to_date_time_string(ts, out);
    out.resize(out.size() + strlen(filename));
    out += filename;
}

const char *log_level_to_string(log_level_t level)
{
    switch(level)
    {
    case ERROR:
        return "ERROR";
    case WARNING:
        return "WARNING";
    case INFO:
        return "INFO";
    case DEBUG:
        return "DEBUG";
    default:
        return "Unknown";
    }
}

void Logger::log(log_level_t level, const char *fmt, ...)
{
    Message msg;
    fill_message_common_parameters(level,msg);

    std::va_list args;
    va_start(args, fmt);

    for (const char *s = fmt; *s != '\0'; ++s) // list all specifiers
    {
        switch(*s)
        {
        case '%':
            switch(*++s) // read format symbol
            {
            case 'd':
            case 'i':
                msg.message.append(std::to_string(va_arg(args, int))); // signed decimal integer
                continue;
            case 'F':
            case 'f':
                msg.message.append(std::to_string(va_arg(args, double))); // decimal floating point
                continue;
            case 's':
                msg.message.append(std::string(va_arg(args, const char*))); // string of characters
                continue;
            case 'c':
                msg.message.push_back(static_cast<char>(va_arg(args, int))); // character
                continue;
            case '%':
                msg.message.append("%"); // special character
                continue;
            case 'x':
            case 'X':
                msg.message.append(to_hex_string(va_arg(args, unsigned int))); // unsigned hexadecimal integer
                continue;
            case 'u':
                msg.message.append(std::to_string(va_arg(args, unsigned int))); // unsigned decimal integer
                continue;
            }
        case '\n':
            msg.message.append("\n"); // special character
            continue;
        case '\t':
            msg.message.append("\t"); // special character
            continue;
        default:
            msg.message.push_back(*s);
        }
    }
    va_end(args);
    m_queuePtr.get()->push(msg);
}

Logger &Logger::operator<<(char &v)
{
    Message msg;
    fill_message_common_parameters(default_level(), msg);
    msg.message.push_back(static_cast<char>(v));
    m_queuePtr.get()->push(msg);
    return *this;
}

Logger &Logger::operator<<(char *v)
{
    Message msg;
    fill_message_common_parameters(default_level(), msg);
    msg.message.append(v);
    m_queuePtr.get()->push(msg);
    return *this;
}

Logger &Logger::operator<<(const char *v)
{
    return operator<<(const_cast<char *>(v));
}

Logger &Logger::operator<<(std::string &v)
{
    return operator<<(v.c_str());
}

Logger &Logger::operator<<(std::vector<char> &v)
{
    Message msg;
    fill_message_common_parameters(default_level(), msg);

    msg.message.append("{ ");
    for (std::size_t i = 0; i < v.size() - 1; ++i)
    {
        if (i && i%16==0)
            msg.message.append("\n");
        msg.message.push_back(static_cast<char>(v[i]));
        msg.message.append(", ");
    }
    msg.message.push_back(static_cast<char>(v[v.size()-1]));
    msg.message.append(" }\n");
    m_queuePtr.get()->push(msg);
    return *this;
}

Handler::Handler(
        const char *root,
        log_level_t maxLevel,
        std::ostream &stream,
        std::error_code &ec
    ):
    m_root{root},
    m_maxLevel{maxLevel},
    m_queuePtr{std::make_shared<SafeQueue<Message>>()},
    m_stream{stream}
{
    if (root == nullptr) {
        ec = make_system_error(EFAULT);
        return;
    }
    if (s_init) {
        ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_SINGLE_INSTANCE);
        return;
    }

    std::filesystem::create_directories(root);

    if (!std::filesystem::is_directory(m_root)) {
        ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_NOT_DIRECTORY);
        return;
    }

    s_init = true;
    ec.clear();
}

void Handler::output_log(const Message &msg, std::ostream &out)
{
    if (msg.format & (1 << LEVEL_BIT))
        out << "[" << log_level_to_string(msg.logLevel) << "] ";
    if (msg.format & (1 << TIMESTAMP_BIT)) {
        std::string ts;
        timestamp_to_date_time_string(msg.timestamp, ts);
        out << ts << " ";
    }
    if (msg.format & (1 << THREAD_ID_BIT)) {
        std::stringstream ss;
        ss << "0x" << std::hex << msg.threadId;
        out << "thread_id: " << ss.str() << " ";
    }
    out << msg.message;
}

void Handler::process()
{
    m_queuePtr.get()->wait_wail_empty_for(1); // wait for 1 second

    if (m_queuePtr.get()->empty())
        return;

    Message msg = m_queuePtr.get()->front();
    m_queuePtr.get()->pop();

    if (msg.logLevel > m_maxLevel || msg.flags == FLAGS_OUTPUT_TO_NOWHERE)
        return;

    if (msg.flags & (1 << OUTPUT_TO_FILE_BIT)) {

        std::filesystem::path filePath = m_root / msg.filename;

        std::ofstream ofs(filePath, std::ofstream::out | std::ofstream::app);

        if (ofs.is_open())
        {
            output_log(msg, ofs);
            ofs.close();
        }
    }
    if (msg.flags & (1 << OUTPUT_TO_STREAM_BIT)) {
        output_log(msg, m_stream);
    }
}

}
