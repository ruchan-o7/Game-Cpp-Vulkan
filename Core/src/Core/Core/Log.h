#pragma once
#include <memory>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace FooGame
{
    class Log
    {
        public:
            static void Init();
            static std::shared_ptr<spdlog::logger>& GetCoreLogger()
            {
                return s_CoreLogger;
            }
            static std::shared_ptr<spdlog::logger>& GetClientLogger()
            {
                return s_ClientLogger;
            }

        private:
            static std::shared_ptr<spdlog::logger> s_CoreLogger;
            static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}  // namespace FooGame

template <typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
    return os << glm::to_string(vector);
}

template <typename OStream, glm::length_t C, glm::length_t R, typename T,
          glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
    return os << glm::to_string(matrix);
}

template <typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
    return os << glm::to_string(quaternion);
}

// Core log macros
#define FOO_CORE_TRACE(...) ::FooGame::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define FOO_CORE_INFO(...)  ::FooGame::Log::GetCoreLogger()->info(__VA_ARGS__)
#define FOO_CORE_WARN(...)  ::FooGame::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define FOO_CORE_ERROR(...) ::FooGame::Log::GetCoreLogger()->error(__VA_ARGS__)
#define FOO_CORE_CRITICAL(...) \
    ::FooGame::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define FOO_TRACE(...) ::FooGame::Log::GetClientLogger()->trace(__VA_ARGS__)
#define FOO_INFO(...)  ::FooGame::Log::GetClientLogger()->info(__VA_ARGS__)
#define FOO_WARN(...)  ::FooGame::Log::GetClientLogger()->warn(__VA_ARGS__)
#define FOO_ERROR(...) ::FooGame::Log::GetClientLogger()->error(__VA_ARGS__)
#define FOO_CRITICAL(...) \
    ::FooGame::Log::GetClientLogger()->critical(__VA_ARGS__)
