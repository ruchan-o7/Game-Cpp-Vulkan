#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include "../Base.h"
#include "src/Log.h"
namespace FooGame
{
    using FloatingPointMicroSeconds = std::chrono::duration<double, std::micro>;

    struct ProfileResult
    {
            String Name;
            FloatingPointMicroSeconds Start;
            std::chrono::microseconds Elapsed;
            std::thread::id ThreadId;
    };

    struct InstrumentationSession
    {
            String Name;
    };

    class Instrumentor
    {
            DELETE_COPY_MOVE(Instrumentor);

        public:
            void BeginSession(const String& name, const String& filePath = "results.json")
            {
                std::lock_guard lock(m_Mutex);
                if (m_pCurrentSession)
                {
                    if (Log::GetCoreLogger())
                    {
                        FOO_CORE_ERROR(
                            "Profiler::BeginSession('{0}') when session '{1}' already open.", name,
                            m_pCurrentSession->Name);
                    }
                    InternalEndSession();
                }
                m_OutputStream.open(filePath);
                if (m_OutputStream.is_open())
                {
                    m_pCurrentSession = new InstrumentationSession({name});
                    WriteHeader();
                }
                else
                {
                    if (Log::GetCoreLogger())
                    {
                        FOO_CORE_ERROR("Instrumentor could not open result file '{0}'.", filePath);
                    }
                }
            }

            void EndSession()
            {
                std::lock_guard lock(m_Mutex);
                InternalEndSession();
            }
            void WriteProfile(const ProfileResult& result)
            {
                std::stringstream json;
                json << std::setprecision(3) << std::fixed;
                json << ",{";
                json << "\"cat\":\"function\",";
                json << "\"dur\":" << (result.Elapsed.count()) << ',';
                json << "\"name\":\"" << result.Name << "\",";
                json << "\"ph\":\"X\",";
                json << "\"pid\":0,";
                json << "\"tid\":" << result.ThreadId << ",";
                json << "\"ts\":" << result.Start.count();
                json << "}";
                std::lock_guard lock(m_Mutex);
                if (m_pCurrentSession)
                {
                    m_OutputStream << json.str();
                    m_OutputStream.flush();
                }
            }

            static Instrumentor& Get()
            {
                static Instrumentor instance;
                return instance;
            }

        private:
            Instrumentor() : m_pCurrentSession(nullptr) {}
            ~Instrumentor() { EndSession(); }

            void WriteHeader()
            {
                m_OutputStream << "{\"otherData\":{},\"traceEvents\":[{}";
                m_OutputStream.flush();
            }
            void WriteFooter()
            {
                m_OutputStream << "]}";
                m_OutputStream.flush();
            }

            void InternalEndSession()
            {
                if (m_pCurrentSession)
                {
                    WriteFooter();
                    m_OutputStream.close();
                    delete m_pCurrentSession;
                    m_pCurrentSession = nullptr;
                }
            }

        private:
            InstrumentationSession* m_pCurrentSession;
            std::mutex m_Mutex;
            std::ofstream m_OutputStream;
    };

    class InstrumentationTimer
    {
        public:
            InstrumentationTimer(const char* name) : m_Name(name), m_Stopped(false) {}
            ~InstrumentationTimer() {}

            void Stop()
            {
                auto endTimepoint = std::chrono::steady_clock::now();
                auto highResStart = FloatingPointMicroSeconds{m_StartTimepoint.time_since_epoch()};
                auto elapsedTime =
                    std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint)
                        .time_since_epoch() -
                    std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint)
                        .time_since_epoch();

                Instrumentor::Get().WriteProfile(
                    {m_Name, highResStart, elapsedTime, std::this_thread::get_id()});
            }

        private:
            const char* m_Name;
            bool m_Stopped;
            std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    };
    namespace InstrumentorUtils
    {
        template <size_t N>
        struct ChangeResult
        {
                char Data[N];
        };
        template <size_t N, size_t K>
        constexpr auto CleanupOutputString(const char (&expr)[N], const char (&remove)[K])
        {
            ChangeResult<K> result = {};
            size_t srcIndex        = 0;
            size_t dstIndex        = 0;
            while (srcIndex < N)
            {
                size_t matchIndex = 0;
                while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 &&
                       expr[srcIndex + matchIndex] == remove[matchIndex])
                {
                    matchIndex++;
                }
                if (matchIndex == K - 1)
                {
                    srcIndex += matchIndex;
                }
                result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
                srcIndex++;
            }
            return result;
        }
    }  // namespace InstrumentorUtils
#define FOO_PROFILE 0

#if FOO_PROFILE

    // Resolve which function signature macro will be used. Note that this only
    // is resolved when the (pre)compiler starts, so the syntax highlighting
    // could mark the wrong one in your editor!

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || \
    (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define FOO_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define FOO_FUNC_SIG __PRETTY_FUNCTION__

#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define FOO_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || \
    (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define FOO_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define FOO_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define FOO_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define FOO_FUNC_SIG __func__
#else
#define FOO_FUNC_SIG "FOO_FUNC_SIG unknown!"
#endif

#define FOO_PROFILE_BEGIN_SESSION(name, filepath) \
    ::FooGame::Instrumentor::Get().BeginSession(name, filepath)
#define FOO_PROFILE_END_SESSION() ::FooGame::Instrumentor::Get().EndSession()
#define FOO_PROFILE_SCOPE_LINE2(name, line)                                  \
    constexpr auto fixedName##line =                                         \
        ::FooGame::InstrumentorUtils::CleanupOutputString(name, "__cdecl "); \
    ::FooGame::InstrumentationTimer timer##line(fixedName##line.Data)
#define FOO_PROFILE_SCOPE_LINE(name, line) FOO_PROFILE_SCOPE_LINE2(name, line)
#define FOO_PROFILE_SCOPE(name)            FOO_PROFILE_SCOPE_LINE(name, __LINE__)
#define FOO_PROFILE_FUNCTION()             FOO_PROFILE_SCOPE(FOO_FUNC_SIG)
#else
#define FOO_PROFILE_BEGIN_SESSION(name, filepath)
#define FOO_PROFILE_END_SESSION()
#define FOO_PROFILE_SCOPE(name)
#define FOO_PROFILE_FUNCTION()
#endif
}  // namespace FooGame
