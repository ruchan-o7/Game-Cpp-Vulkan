#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
namespace FooGame
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_EngineLogger;
    std::shared_ptr<spdlog::logger> Log::s_EditorLogger;
    std::shared_ptr<spdlog::logger> Log::s_GameLogger;

    void Log::Init(AppType type)
    {
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");

        s_CoreLogger = std::make_shared<spdlog::logger>("[CORE]", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_CoreLogger);
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);

        s_EngineLogger =
            std::make_shared<spdlog::logger>("[ENGINE]", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_EngineLogger);
        s_EngineLogger->set_level(spdlog::level::trace);
        s_EngineLogger->flush_on(spdlog::level::trace);

        switch (type)
        {
            case AppType::Game:
                s_GameLogger =
                    std::make_shared<spdlog::logger>("[GAME]", begin(logSinks), end(logSinks));
                spdlog::register_logger(s_GameLogger);
                s_GameLogger->set_level(spdlog::level::trace);
                s_GameLogger->flush_on(spdlog::level::trace);

                break;
            case AppType::Editor:
                s_EditorLogger =
                    std::make_shared<spdlog::logger>("[EDITOR]", begin(logSinks), end(logSinks));
                spdlog::register_logger(s_EditorLogger);
                s_EditorLogger->set_level(spdlog::level::trace);
                s_EditorLogger->flush_on(spdlog::level::trace);
                break;
        }
    }

}  // namespace FooGame
