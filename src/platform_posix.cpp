#include "platform.hpp"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <sstream>
#include <vector>

namespace tslogger::platform
{

static std::vector<std::string> split_path(const std::string &path)
{
    std::vector<std::string> parts;
    std::string current;
    for (char ch : path) {
        if (ch == '/') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

bool create_directories(const std::string &path, std::error_code &ec)
{
    if (path.empty()) {
        ec = std::make_error_code(std::errc::invalid_argument);
        return false;
    }

    std::string current = (path.front() == '/') ? "/" : "";
    for (const std::string &part : split_path(path)) {
        if (!current.empty() && current.back() != '/') {
            current.push_back('/');
        }
        current += part;

        if (::mkdir(current.c_str(), 0755) == -1 && errno != EEXIST) {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }
    }

    ec.clear();
    return true;
}

bool is_directory(const std::string &path, std::error_code &ec)
{
    struct stat st = {};
    if (::stat(path.c_str(), &st) == -1) {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }

    ec.clear();
    return S_ISDIR(st.st_mode);
}

bool append_to_file(const std::string &path, const std::string &text, std::error_code &ec)
{
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }

    ssize_t writtenTotal = 0;
    const ssize_t expected = static_cast<ssize_t>(text.size());
    while (writtenTotal < expected) {
        const ssize_t written = ::write(fd, text.data() + writtenTotal, expected - writtenTotal);
        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }
            ec = std::error_code(errno, std::generic_category());
            ::close(fd);
            return false;
        }
        writtenTotal += written;
    }

    ::close(fd);
    ec.clear();
    return true;
}

bool localtime_safe(std::time_t ts, std::tm &out)
{
    return ::localtime_r(&ts, &out) != nullptr;
}

std::string thread_id_to_string(std::thread::id id)
{
    std::stringstream ss;
    ss << id;
    return ss.str();
}

} // namespace tslogger::platform
