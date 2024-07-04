#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "spdlog/async.h"
namespace FooGame
{
    std::shared_ptr<spdlog::async_logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::async_logger> Log::s_EngineLogger;
    std::shared_ptr<spdlog::async_logger> Log::s_EditorLogger;
    std::shared_ptr<spdlog::async_logger> Log::s_GameLogger;
    std::shared_ptr<spdlog::details::thread_pool> Log::s_LoggerThreadPool;

    void Log::Init()
    {
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");
        spdlog::init_thread_pool(1024, 2);
        s_LoggerThreadPool = spdlog::thread_pool();

        s_CoreLogger = std::make_shared<spdlog::async_logger>("[CORE]", begin(logSinks),
                                                              end(logSinks), s_LoggerThreadPool,
                                                              spdlog::async_overflow_policy::block);

        spdlog::register_logger(s_CoreLogger);
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);

        s_EngineLogger = std::make_shared<spdlog::async_logger>(
            "[ENGINE]", begin(logSinks), end(logSinks), s_LoggerThreadPool,
            spdlog::async_overflow_policy::block);
        spdlog::register_logger(s_EngineLogger);
        s_EngineLogger->set_level(spdlog::level::trace);
        s_EngineLogger->flush_on(spdlog::level::trace);

        s_GameLogger = std::make_shared<spdlog::async_logger>("[GAME]", begin(logSinks),
                                                              end(logSinks), s_LoggerThreadPool,
                                                              spdlog::async_overflow_policy::block);
        spdlog::register_logger(s_GameLogger);
        s_GameLogger->set_level(spdlog::level::trace);
        s_GameLogger->flush_on(spdlog::level::trace);

        s_EditorLogger = std::make_shared<spdlog::async_logger>(
            "[EDITOR]", begin(logSinks), end(logSinks), s_LoggerThreadPool,
            spdlog::async_overflow_policy::block);
        spdlog::register_logger(s_EditorLogger);
        s_EditorLogger->set_level(spdlog::level::trace);
        s_EditorLogger->flush_on(spdlog::level::trace);
    }

}  // namespace FooGame
