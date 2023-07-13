#ifndef _TS_LOGGER_HPP
#define _TS_LOGGER_HPP
#include <string>
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <ios>
#include <type_traits>
#include <mutex>
#include <string_view>
#include <syncstream>
#include <vector>
#include <array>
#include "safe_queue.hpp"
#include "error.hpp"

namespace tslogger
{

typedef enum {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
} log_level_t;

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

typedef uint8_t line_format_t;

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

typedef uint8_t flags_t;

inline bool is_flags_type(flags_t flags)
{
    return (flags >= FLAGS_OUTPUT_TO_NOWHERE && flags <= FLAGS_OUTPUT_TO_ALL);
}

struct Message {
    log_level_t logLevel;
    std::string message;
    std::thread::id threadId;
    std::string filename;
    line_format_t format;
    flags_t flags;
    time_t timestamp;
};

time_t timestamp();
void timestamp_to_date_time_string(time_t ts, std::string &out);
void add_timestamp_prefix(const char *filename, std::string &out);
const char *log_level_to_string(log_level_t level);
void output_log(const Message &msg, std::ostream &out);

template< typename T >
std::string to_hex_string(T t)
{
    std::stringstream stream;
    stream << "0x" << std::hex << t;
    //stream << "0x" << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex << t;
    return stream.str();
}

class Logger {
public:
    Logger(
            std::shared_ptr<SafeQueue<Message>> queuePtr,
            const char *filename,
            const flags_t flags,
            const line_format_t format = LINE_FORMAT_ALL
        ):
        m_queuePtr{queuePtr},
        m_filename{},
        m_threadId{std::this_thread::get_id()},
        m_flags{flags},
        m_format{format},
        m_defaultLevel{DEBUG}
    {
        if (filename == nullptr) {
            add_timestamp_prefix("_untitled.log", m_filename);
        }
        else {
            m_filename = filename;
        }
        if (!is_flags_type(m_flags)) {
            m_flags = FLAGS_OUTPUT_TO_FILE_ONLY;
        }
        if (!is_line_format_type(m_format)) {
            m_format = LINE_FORMAT_ALL;
        }
    }
    ~Logger()
    {}

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger &operator=(const Logger&) = delete;
    Logger &operator=(Logger&&) = delete;

    Logger &operator<<(const char *v);
    Logger &operator<<(char *v);
    Logger &operator<<(std::string &v);
    Logger &operator<<(std::vector<char> &v);
    Logger &operator<<(char &v);

    void log(log_level_t level, const char *fmt, ...);

    void fill_message_common_parameters(log_level_t level, Message &msg)
    {
        msg.timestamp = timestamp();
        msg.logLevel = level;
        msg.threadId = m_threadId;
        msg.filename = m_filename;
        msg.format = m_format;
        msg.flags = m_flags;
    }

    template <typename T>
    Logger &operator<<(T &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message = std::to_string(v);
        m_queuePtr.get()->push(msg);
        return *this;
    }

    template <typename T>
    Logger &operator<<(std::vector<T> &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message = "{ ";
        for (std::size_t i = 0; i < v.size() - 1; ++i) {
            if (i && i%16==0)
                msg.message += "\n";
            msg.message += std::to_string(v[i]) + ", ";
        }
        msg.message += std::to_string(v[v.size()-1]) + " }\n";
        m_queuePtr.get()->push(msg);
        return *this;
    }

    template <typename std::size_t N>
    Logger &operator<<(std::array<char, N> &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message = "{ ";
        for (std::size_t i = 0; i < N-1; ++i) {
            if (i && i%16==0)
                msg.message += "\n";
            msg.message += static_cast<char>(v[i]); msg.message += ", ";
        }
        msg.message += static_cast<char>(v[N-1]); msg.message += " }\n";
        m_queuePtr.get()->push(msg);
        return *this;
    }

    template <typename T, std::size_t N>
    Logger &operator<<(std::array<T, N> &v)
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message = "{ ";
        for (std::size_t i = 0; i < N-1; ++i) {
            if (i && i%16==0)
                msg.message += "\n";
            msg.message += std::to_string(v[i]) + ", ";
        }
        msg.message += std::to_string(v[N-1]) + " }\n";
        m_queuePtr.get()->push(msg);
        return *this;
    }

    template <typename T, std::size_t N>
    Logger &operator<<(T(&v)[N])
    {
        Message msg;
        fill_message_common_parameters(default_level(), msg);
        msg.message = "{ ";
        for (std::size_t i = 0; i < N-1; ++i) {
            if (i && i%16==0)
                msg.message += "\n";
            msg.message += std::to_string(v[i]) + ", ";
        }
        msg.message += std::to_string(v[N-1]) + " }\n";
        m_queuePtr.get()->push(msg);
        return *this;
    }

    template <typename T, std::size_t N>
    Logger &operator<<(const T(&v)[N])
    {
        return operator<<(const_cast<T[N]>(v));
    }

    void filename(const char *filename)
    {
        m_filename = filename;
    }

    const char *filename() const
    {
        return m_filename.c_str();
    }

    flags_t flags() const
    {
        return m_flags;
    }

    void flags(flags_t flags)
    {
        m_flags = flags;
    }

    void format(line_format_t format)
    {
        m_format = format;
    }

    line_format_t format() const
    {
        return m_format;
    }

    void default_level(log_level_t level)
    {
        m_defaultLevel = level;
    }

    log_level_t default_level() const
    {
        return m_defaultLevel;
    }

private:
    std::shared_ptr<SafeQueue<Message>> m_queuePtr;
    std::string m_filename;
    std::thread::id m_threadId;
    flags_t m_flags;
    line_format_t m_format;
    log_level_t m_defaultLevel;
};

class Handler {
public:
    Handler(
            const char *root,
            log_level_t maxLevel,
            std::ostream &stream,
            std::error_code &ec
        );
    ~Handler()
    {}
    Handler(const Logger&) = delete;
    Handler(Logger&&) = delete;
    Handler &operator=(const Logger&) = delete;
    Handler &operator=(Logger&&) = delete;

    std::shared_ptr<SafeQueue<Message>> get_queue_ptr()
    {
        return m_queuePtr;
    }

    void process();

    void root(std::filesystem::path root, std::error_code &ec)
    {
        const std::lock_guard<std::mutex> lg(s_mutex);
        m_root = root;
        std::filesystem::create_directories(root);
        if (!std::filesystem::is_directory(root)) {
            ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_NOT_DIRECTORY);
            return;
        }
    }

    void root(const char *_root, std::error_code &ec)
    {
        return this->root(std::filesystem::path(_root), ec);
    }

    const std::filesystem::path root() const
    {
        const std::lock_guard<std::mutex> lg(s_mutex);
        const std::filesystem::path root = m_root;
        return root;
    }

    void max_level(log_level_t level)
    {
        const std::lock_guard<std::mutex> lg(s_mutex);
        m_maxLevel = level;
    }

    const log_level_t max_level() const
    {
        const std::lock_guard<std::mutex> lg(s_mutex);
        const log_level_t level = m_maxLevel;
        return level;
    }

private:
    static void output_log(const Message &msg, std::ostream &out);

private:
    std::filesystem::path m_root;
    log_level_t m_maxLevel;
    std::shared_ptr<SafeQueue<Message>> m_queuePtr;
    std::ostream &m_stream;
    static bool s_init;
    static std::mutex s_mutex;
};

#ifdef USE_TS_LOGGER
#define LOG(obj, logLevel, ...) obj.log(logLevel, __VA_ARGS__)
#else
#define LOG(obj, logLevel, ...)
#endif

#define ENTER_LOG(obj, logLevel) LOG(obj, logLevel, "<<< Entering\n")
#define EXIT_LOG(obj, logLevel) LOG(obj, logLevel, ">>> Exiting\n")

} // tslogger

#endif // _TS_LOGGER_HPP
