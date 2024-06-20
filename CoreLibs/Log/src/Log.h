#pragma once
#include <memory>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/async.h>

#pragma warning(pop)

namespace FooGame
{
    enum class AppType
    {
        Game,
        Editor,
        Core,
    };
    // TODO: Make it thread safe
    class Log
    {
        public:
            static void Init(AppType type);
            static std::shared_ptr<spdlog::async_logger>& GetCoreLogger() { return s_CoreLogger; }
            static std::shared_ptr<spdlog::async_logger>& GetEngineLogger()
            {
                return s_EngineLogger;
            }
            static std::shared_ptr<spdlog::async_logger>& GetEditorLogger()
            {
                return s_EditorLogger;
            }
            static std::shared_ptr<spdlog::async_logger>& GetGameLogger() { return s_GameLogger; }

        private:
            static std::shared_ptr<spdlog::async_logger> s_CoreLogger;
            static std::shared_ptr<spdlog::async_logger> s_EngineLogger;
            static std::shared_ptr<spdlog::async_logger> s_EditorLogger;
            static std::shared_ptr<spdlog::async_logger> s_GameLogger;
            static std::shared_ptr<spdlog::details::thread_pool> s_LoggerThreadPool;
    };
}  // namespace FooGame

// Core log macros
#define FOO_CORE_TRACE(...)    ::FooGame::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define FOO_CORE_INFO(...)     ::FooGame::Log::GetCoreLogger()->info(__VA_ARGS__)
#define FOO_CORE_WARN(...)     ::FooGame::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define FOO_CORE_ERROR(...)    ::FooGame::Log::GetCoreLogger()->error(__VA_ARGS__)
#define FOO_CORE_CRITICAL(...) ::FooGame::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Engine log macros
#define FOO_ENGINE_TRACE(...)    ::FooGame::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define FOO_ENGINE_INFO(...)     ::FooGame::Log::GetEngineLogger()->info(__VA_ARGS__)
#define FOO_ENGINE_WARN(...)     ::FooGame::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define FOO_ENGINE_ERROR(...)    ::FooGame::Log::GetEngineLogger()->error(__VA_ARGS__)
#define FOO_ENGINE_CRITICAL(...) ::FooGame::Log::GetEngineLogger()->critical(__VA_ARGS__)

// Editor log macros
#define FOO_EDITOR_TRACE(...)    ::FooGame::Log::GetEditorLogger()->trace(__VA_ARGS__)
#define FOO_EDITOR_INFO(...)     ::FooGame::Log::GetEditorLogger()->info(__VA_ARGS__)
#define FOO_EDITOR_WARN(...)     ::FooGame::Log::GetEditorLogger()->warn(__VA_ARGS__)
#define FOO_EDITOR_ERROR(...)    ::FooGame::Log::GetEditorLogger()->error(__VA_ARGS__)
#define FOO_EDITOR_CRITICAL(...) ::FooGame::Log::GetEditorLogger()->critical(__VA_ARGS__)

// Game log macros
#define FOO_GAME_TRACE(...)    ::FooGame::Log::GetGameLogger()->trace(__VA_ARGS__)
#define FOO_GAME_INFO(...)     ::FooGame::Log::GetGameLogger()->info(__VA_ARGS__)
#define FOO_GAME_WARN(...)     ::FooGame::Log::GetGameLogger()->warn(__VA_ARGS__)
#define FOO_GAME_ERROR(...)    ::FooGame::Log::GetGameLogger()->error(__VA_ARGS__)
#define FOO_GAME_CRITICAL(...) ::FooGame::Log::GetGameLogger()->critical(__VA_ARGS__)
