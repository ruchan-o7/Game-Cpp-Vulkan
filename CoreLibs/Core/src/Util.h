#include <functional>
#include <iostream>
#pragma once
namespace FooGame
{
    class Defer
    {
        public:
            explicit Defer(const std::function<void()>& f) : m_Func(f) {}
            ~Defer() { m_Func(); }

        private:
            std::function<void()> m_Func;
    };
    template <typename... Args>
    inline static std::string StrFormat(const std::string& format, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
        if (size_s <= 0)
        {
            std::cerr << "[ERROR] | "
                      << " Error during string format: " << format << std::endl;
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf{new char[size]};
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
}  // namespace FooGame