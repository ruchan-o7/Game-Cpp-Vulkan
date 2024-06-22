#include <functional>
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
}  // namespace FooGame