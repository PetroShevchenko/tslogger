#include <cstdarg>
#include <ctime>

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
    std::tm tmValue{};
    if (!platform::localtime_safe(ts, tmValue)) {
        out.clear();
        return;
    }

    char buf[20] = {};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmValue);
    out = buf;
}

void add_timestamp_prefix(const char *filename, std::string &out)
{
    time_t ts = timestamp();
    timestamp_to_date_time_string(ts, out);
    out += filename;
}

const char *log_level_to_string(log_level_t level)
{
    switch (level) {
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
    fill_message_common_parameters(level, msg);

    std::va_list args;
    va_start(args, fmt);

    for (const char *s = fmt; *s != '\0'; ++s) {
        switch (*s) {
        case '%':
            switch (*++s) {
            case 'd':
            case 'i':
                msg.message.append(std::to_string(va_arg(args, int)));
                continue;
            case 'F':
            case 'f':
                msg.message.append(std::to_string(va_arg(args, double)));
                continue;
            case 's':
                msg.message.append(std::string(va_arg(args, const char *)));
                continue;
            case 'c':
                msg.message.push_back(static_cast<char>(va_arg(args, int)));
                continue;
            case '%':
                msg.message.append("%");
                continue;
            case 'x':
            case 'X':
                msg.message.append(to_hex_string(va_arg(args, unsigned int)));
                continue;
            case 'u':
                msg.message.append(std::to_string(va_arg(args, unsigned int)));
                continue;
            default:
                msg.message.push_back('%');
                msg.message.push_back(*s);
                continue;
            }
        case '\n':
            msg.message.append("\n");
            continue;
        case '\t':
            msg.message.append("\t");
            continue;
        default:
            msg.message.push_back(*s);
        }
    }
    va_end(args);
    m_queuePtr->push(msg);
}

Logger &Logger::operator<<(char &v)
{
    Message msg;
    fill_message_common_parameters(default_level(), msg);
    msg.message.push_back(v);
    m_queuePtr->push(msg);
    return *this;
}

Logger &Logger::operator<<(char *v)
{
    Message msg;
    fill_message_common_parameters(default_level(), msg);
    msg.message.append(v == nullptr ? "<null>" : v);
    m_queuePtr->push(msg);
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
    if (v.empty()) {
        msg.message.append("}");
        m_queuePtr->push(msg);
        return *this;
    }
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i && i % 16 == 0)
            msg.message.append("\n");
        msg.message.push_back(static_cast<char>(v[i]));
        msg.message.append(i + 1 < v.size() ? ", " : " }");
    }
    m_queuePtr->push(msg);
    return *this;
}

Handler::Handler(const char *root, log_level_t maxLevel, std::ostream &stream, std::error_code &ec)
    :
      m_root{root == nullptr ? "" : root},
      m_maxLevel{maxLevel},
      m_queuePtr{std::make_shared<SafeQueue<Message>>()},
      m_stream{stream}
{
    if (root == nullptr) {
        ec = make_system_error(EFAULT);
        return;
    }

    std::lock_guard<std::mutex> lg(s_mutex);
    if (s_init) {
        ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_SINGLE_INSTANCE);
        return;
    }

    if (!platform::create_directories(m_root, ec) || !platform::is_directory(m_root, ec)) {
        if (!ec) {
            ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_NOT_DIRECTORY);
        }
        return;
    }

    s_init = true;
    ec.clear();
}

Handler::~Handler()
{
    std::lock_guard<std::mutex> lg(s_mutex);
    s_init = false;
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
        out << "thread_id: " << platform::thread_id_to_string(msg.threadId) << " ";
    }
    out << msg.message;
}

void Handler::process()
{
    m_queuePtr->wait_wail_empty_for(1);

    auto msgOpt = m_queuePtr->pop();
    if (!msgOpt.has_value()) {
        return;
    }
    const Message &msg = *msgOpt;

    if (msg.logLevel > max_level() || msg.flags == FLAGS_OUTPUT_TO_NOWHERE)
        return;

    if (msg.flags & (1 << OUTPUT_TO_FILE_BIT)) {
        std::error_code ec;
        const std::string filePath = m_root + "/" + msg.filename;
        const std::string line = [&msg]() {
            std::ostringstream oss;
            output_log(msg, oss);
            return oss.str();
        }();

        platform::append_to_file(filePath, line, ec);
    }
    if (msg.flags & (1 << OUTPUT_TO_STREAM_BIT)) {
        output_log(msg, m_stream);
    }
}

void Handler::root(std::string rootValue, std::error_code &ec)
{
    const std::lock_guard<std::mutex> lg(s_mutex);
    if (!platform::create_directories(rootValue, ec) || !platform::is_directory(rootValue, ec)) {
        if (!ec) {
            ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_NOT_DIRECTORY);
        }
        return;
    }

    m_root = std::move(rootValue);
    ec.clear();
}

std::string Handler::root() const
{
    const std::lock_guard<std::mutex> lg(s_mutex);
    return m_root;
}

void Handler::max_level(log_level_t level)
{
    const std::lock_guard<std::mutex> lg(s_mutex);
    m_maxLevel = level;
}

log_level_t Handler::max_level() const
{
    const std::lock_guard<std::mutex> lg(s_mutex);
    return m_maxLevel;
}

} // namespace tslogger
