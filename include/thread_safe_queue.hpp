// include/thread_safe_queue.hpp
#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() {}
    
    void enqueue(const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
        m_cond_var.notify_one();
    }
    
    bool dequeue(T& result) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty() && !m_shutdown) {
            m_cond_var.wait(lock);
        }
        if (m_shutdown && m_queue.empty()) {
            return false;
        }
        result = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    
    void shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shutdown = true;
        m_cond_var.notify_all();
    }
    
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond_var;
    bool m_shutdown = false;
};

#endif // THREAD_SAFE_QUEUE_HPP
