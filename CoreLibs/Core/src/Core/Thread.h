#pragma once
#include <pch.h>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
namespace FooGame
{
    class Thread
    {
        public:
            static std::unique_ptr<Thread> Create();
            ~Thread();

        public:
            void AddTask(std::function<void()> func);
            void Wait();

        private:
            Thread();
            void QueueLoop();

        private:
            std::thread m_WorkerThread;
            std::queue<std::function<void()>> m_TaskQueue;
            std::mutex m_QueueMutex;
            std::condition_variable m_Condition;
            bool m_IsRunning;
    };

}  // namespace FooGame