#ifndef _SAFE_QUEUE_HPP
#define _SAFE_QUEUE_HPP

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>

#include "logger_error.hpp"

template<typename T>
struct SafeQueue
{
    SafeQueue() = default;
    ~SafeQueue() = default;

    void push(T value);

    std::optional<T> pop();
    std::optional<T> front();
    bool empty();
    size_t size();
    void wait_wail_empty();
    void wait_wail_empty_for(size_t seconds);

private:
    std::queue<T> m_queue_;
    std::mutex m_mutex_;
    std::condition_variable m_cv_;
};

template<typename T>
struct SafeQueue<T *>
{
    SafeQueue() = default;
    ~SafeQueue() = default;

    void push(T *value, std::error_code &ec);
    T *pop();
    T *front();
    bool empty();
    size_t size();
    void wait_wail_empty();
    void wait_wail_empty_for(size_t seconds);

private:
    std::queue<T *> m_queue_;
    std::mutex m_mutex_;
    std::condition_variable m_cv_;
};

template<typename T>
void SafeQueue<T *>::push(T *value, std::error_code &ec)
{
    if (value == nullptr) {
        ec = tslogger::make_system_error(EFAULT);
        return;
    }
    std::unique_lock<std::mutex> ul(m_mutex_);
    m_queue_.push(value);
    ul.unlock();
    m_cv_.notify_one();
}

template<typename T>
T *SafeQueue<T *>::pop()
{
    T *value = nullptr;
    std::unique_lock<std::mutex> ul(m_mutex_);

    if (!m_queue_.empty()) {
        value = m_queue_.front();
        m_queue_.pop();
    }
    return value;
}

template<typename T>
T *SafeQueue<T *>::front()
{
    T *value = nullptr;
    std::lock_guard<std::mutex> lg(m_mutex_);

    if (!m_queue_.empty()) {
        value = m_queue_.front();
}

    return value;
}

template<typename T>
bool SafeQueue<T *>::empty()
{
    std::lock_guard<std::mutex> lg(m_mutex_);
    return m_queue_.empty();
}

template<typename T>
size_t SafeQueue<T *>::size()
{
    std::lock_guard<std::mutex> lg(m_mutex_);
    return m_queue_.size();
}

template<typename T>
void SafeQueue<T *>::wait_wail_empty()
{
    std::unique_lock<std::mutex> ul(m_mutex_);
    if (!m_queue_.empty()) {
        return;
}
    m_cv_.wait(ul, [this] { return !m_queue_.empty(); });
}

template<typename T>
void SafeQueue<T *>::wait_wail_empty_for(size_t seconds)
{
    std::unique_lock<std::mutex> ul(m_mutex_);
    if (!m_queue_.empty()) {
        return;
}
    m_cv_.wait_for(ul, std::chrono::seconds(seconds), [this] { return !m_queue_.empty(); });
}

template<typename T>
void SafeQueue<T>::push(T value)
{
    std::unique_lock<std::mutex> ul(m_mutex_);
    m_queue_.push(std::move(value));
    ul.unlock();
    m_cv_.notify_one();
}

template<typename T>
std::optional<T> SafeQueue<T>::pop()
{
    std::unique_lock<std::mutex> ul(m_mutex_);

    if (m_queue_.empty()) {
        return std::nullopt;
    }

    T value = std::move(m_queue_.front());
    m_queue_.pop();
    return value;
}

template<typename T>
std::optional<T> SafeQueue<T>::front()
{
    std::lock_guard<std::mutex> lg(m_mutex_);

    if (m_queue_.empty()) {
        return std::nullopt;
    }

    return m_queue_.front();
}

template<typename T>
bool SafeQueue<T>::empty()
{
    std::lock_guard<std::mutex> lg(m_mutex_);
    return m_queue_.empty();
}

template<typename T>
size_t SafeQueue<T>::size()
{
    std::lock_guard<std::mutex> lg(m_mutex_);
    return m_queue_.size();
}

template<typename T>
void SafeQueue<T>::wait_wail_empty()
{
    std::unique_lock<std::mutex> ul(m_mutex_);
    if (!m_queue_.empty()) {
        return;
}
    m_cv_.wait(ul, [this] { return !m_queue_.empty(); });
}

template<typename T>
void SafeQueue<T>::wait_wail_empty_for(size_t seconds)
{
    std::unique_lock<std::mutex> ul(m_mutex_);
    if (!m_queue_.empty()) {
        return;
}
    m_cv_.wait_for(ul, std::chrono::seconds(seconds), [this] { return !m_queue_.empty(); });
}

#endif
