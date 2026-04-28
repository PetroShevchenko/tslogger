#ifndef _TS_LOGGER_HPP
#define _TS_LOGGER_HPP

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <ios>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include "logger_error.hpp"
#include "platform.hpp"
#include "safe_queue.hpp"

namespace tslogger
{

enum log_level_t {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
};

enum {
    LEVEL_BIT = 0,
    TIMESTAMP_BIT = 1,
    THREAD_ID_BIT = 2,
};

enum {
    LINE_FORMAT_MSG_ONLY = 0,
    LINE_FORMAT_LEVEL_ONLY = (1 << LEVEL_BIT),
    LINE_FORMAT_LEVEL_AND_THREAD_ID = (1 << LEVEL_BIT) | (1 << THREAD_ID_BIT),
    LINE_FORMAT_ALL = (1 << LEVEL_BIT) | (1 << TIMESTAMP_BIT) | (1 << THREAD_ID_BIT),
};

using line_format_t = uint8_t;

inline bool is_line_format_type(line_format_t format)
{
    return (format >= LINE_FORMAT_MSG_ONLY && format <= LINE_FORMAT_ALL);
}

enum {
    OUTPUT_TO_FILE_BIT = 0,
    OUTPUT_TO_STREAM_BIT = 1,
};

enum {
    FLAGS_OUTPUT_TO_NOWHERE = 0,
    FLAGS_OUTPUT_TO_FILE_ONLY = (1 << OUTPUT_TO_FILE_BIT),
    FLAGS_OUTPUT_TO_STREAM_ONLY = (1 << OUTPUT_TO_STREAM_BIT),
    FLAGS_OUTPUT_TO_ALL = (1 << OUTPUT_TO_FILE_BIT) | (1 << OUTPUT_TO_STREAM_BIT),
};

using flags_t = uint8_t;

inline bool is_flags_type(flags_t flags)
{
    return (flags >= FLAGS_OUTPUT_TO_NOWHERE && flags <= FLAGS_OUTPUT_TO_ALL);
}

struct Message {
    log_level_t logLevel_;
    std::string message_;
    std::thread::id threadId_;
    std::string filename_;
    line_format_t format_;
    flags_t flags_;
    time_t timestamp_;
};

time_t timestamp();
void timestamp_to_date_time_string(time_t ts, std::string &out);
void add_timestamp_prefix(const char *filename, std::string &out);
const char *log_level_to_string(log_level_t level);

template<typename T>
std::string to_hex_string(T t)
{
    std::stringstream stream;
    stream << "0x" << std::hex << t;
    return stream.str();
}

class Logger {
public:
    Logger(
        std::shared_ptr<SafeQueue<Message>> queuePtr,
        const char *filename,
        flags_t flags,
        line_format_t format = LINE_FORMAT_ALL)
        :
          m_queuePtr_{std::move(queuePtr)},
          m_filename_{},
          m_flags_{flags},
          m_format_{format},
          m_defaultLevel_{DEBUG}
    {
        if (filename == nullptr) {
            add_timestamp_prefix("_untitled.log", m_filename_);
        } else {
            m_filename_.append(filename);
        }
        if (!is_flags_type(m_flags_)) {
            m_flags_ = FLAGS_OUTPUT_TO_FILE_ONLY;
        }
        if (!is_line_format_type(m_format_)) {
            m_format_ = LINE_FORMAT_ALL;
        }
    }

    ~Logger() = default;

    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;

    Logger &operator<<(const char *v);
    Logger &operator<<(char *v);
    Logger &operator<<(std::string &v);
    Logger &operator<<(std::vector<char> &v);
    Logger &operator<<(char &v);

    void log(log_level_t level, const char *fmt, ...);

    void fill_message_common_parameters(log_level_t level, Message &msg)
    {
        msg.timestamp_ = timestamp();
        msg.logLevel_ = level;
        msg.threadId_ = std::this_thread::get_id();
        msg.filename_ = m_filename_;
        msg.format_ = m_format_;
        msg.flags_ = m_flags_;
    }

    template<typename T>
    Logger &operator<<(T &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message_.append(std::to_string(v));
        m_queuePtr_->push(msg);
        return *this;
    }

    template<typename T>
    Logger &operator<<(std::vector<T> &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message_.append("{ ");
        if (v.empty()) {
            msg.message_.append("}");
            m_queuePtr_->push(msg);
            return *this;
        }
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (i && i % 16 == 0) {
                msg.message_.append("\n");
}
            msg.message_.append(std::to_string(v[i]));
            msg.message_.append(i + 1 < v.size() ? ", " : " }");
        }
        m_queuePtr_->push(msg);
        return *this;
    }

    template<std::size_t N>
    Logger &operator<<(std::array<char, N> &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message_.append("{ ");
        for (std::size_t i = 0; i < N; ++i) {
            if (i && i % 16 == 0) {
                msg.message_.append("\n");
}
            msg.message_.push_back(static_cast<char>(v[i]));
            msg.message_.append(i + 1 < N ? ", " : " }");
        }
        m_queuePtr_->push(msg);
        return *this;
    }

    template<typename T, std::size_t N>
    Logger &operator<<(std::array<T, N> &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message_.append("{ ");
        for (std::size_t i = 0; i < N; ++i) {
            if (i && i % 16 == 0) {
                msg.message_.append("\n");
}
            msg.message_.append(std::to_string(v[i]));
            msg.message_.append(i + 1 < N ? ", " : " }");
        }
        m_queuePtr_->push(msg);
        return *this;
    }

    template<typename T, std::size_t N>
    Logger &operator<<(T (&v)[N])
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message_.append("{ ");
        for (std::size_t i = 0; i < N; ++i) {
            if (i && i % 16 == 0) {
                msg.message_.append("\n");
}
            msg.message_.append(std::to_string(v[i]));
            msg.message_.append(i + 1 < N ? ", " : " }");
        }
        m_queuePtr_->push(msg);
        return *this;
    }

    template<typename T, std::size_t N>
    Logger &operator<<(const T (&v)[N])
    {
        return operator<<(const_cast<T (&)[N]>(v));
    }

    void filename(const char *filename)
    {
        if (filename != nullptr) {
            m_filename_ = filename;
        }
    }

    const char *filename() const { return m_filename_.c_str(); }

    flags_t flags() const { return m_flags_; }

    void flags(flags_t flags) { m_flags_ = flags; }

    void format(line_format_t format) { m_format_ = format; }

    line_format_t format() const { return m_format_; }

    void default_level(log_level_t level) { m_defaultLevel_ = level; }

    log_level_t default_level() const { return m_defaultLevel_; }

    std::shared_ptr<SafeQueue<Message>> queue_ptr() { return m_queuePtr_; }

private:
    std::shared_ptr<SafeQueue<Message>> m_queuePtr_;
    std::string m_filename_;
    flags_t m_flags_;
    line_format_t m_format_;
    log_level_t m_defaultLevel_;
};

class Handler {
public:
    Handler(const char *root, log_level_t maxLevel, std::ostream &stream, std::error_code &ec);
    ~Handler();

    Handler(const Handler &) = delete;
    Handler(Handler &&) = delete;
    Handler &operator=(const Handler &) = delete;
    Handler &operator=(Handler &&) = delete;

    std::shared_ptr<SafeQueue<Message>> get_queue_ptr() { return m_queuePtr_; }

    void process();

    void root(std::string root, std::error_code &ec);
    void root(const char *_root, std::error_code &ec)
    {
        this->root(std::string(_root == nullptr ? "" : _root), ec);
    }

    std::string root() const;

    void max_level(log_level_t level);

    log_level_t max_level() const;

private:
    static void output_log(const Message &msg, std::ostream &out);

private:
    std::string m_root_;
    log_level_t m_maxLevel_;
    std::shared_ptr<SafeQueue<Message>> m_queuePtr_;
    std::ostream &m_stream_;
    static bool s_init;
    static std::mutex s_mutex;
};

#ifdef USE_TS_LOGGER
#define LOG(obj, logLevel, ...) obj.log(logLevel, __VA_ARGS__)
#else
#define LOG(obj, logLevel, ...)
#endif

#define ENTER_LOG(obj, logLevel) LOG(obj, logLevel, "%s:%d <<< Entering\n", __FILE__, __LINE__)
#define EXIT_LOG(obj, logLevel) LOG(obj, logLevel, "%s:%d >>> Exiting\n", __FILE__, __LINE__)

} // namespace tslogger

#endif // _TS_LOGGER_HPP
