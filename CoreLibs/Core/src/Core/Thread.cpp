#include "Thread.h"
#include <mutex>
#include <utility>
namespace FooGame
{
    std::unique_ptr<Thread> Thread::Create()
    {
        auto* t = new Thread();
        return std::unique_ptr<Thread>(t);
    }

    Thread::Thread() : m_WorkerThread(&Thread::QueueLoop, this), m_IsRunning(true)
    {
    }
    Thread::~Thread()
    {
        if (m_WorkerThread.joinable())
        {
            Wait();
            m_QueueMutex.lock();
            m_IsRunning = false;
            m_Condition.notify_one();
            m_QueueMutex.unlock();
            m_WorkerThread.join();
        }
    }
    void Thread::Wait()
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_Condition.wait(lock, [this] { return m_TaskQueue.empty(); });
    }
    void Thread::QueueLoop()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Condition.wait(lock, [this] { return !m_TaskQueue.empty() || !m_IsRunning; });
                if (!m_IsRunning && m_TaskQueue.empty())
                {
                    break;
                }
                task = std::move(m_TaskQueue.front());
            }
            task();
            {
                std::lock_guard<std::mutex> lock(m_QueueMutex);
                m_TaskQueue.pop();
                m_Condition.notify_one();
            }
        }
    }
    void Thread::AddTask(std::function<void()> func)
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_TaskQueue.push(func);
        m_Condition.notify_one();
    }
}  // namespace FooGame