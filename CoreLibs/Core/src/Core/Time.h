#pragma once
#include <cstdint>
namespace FooGame
{
    class Time
    {
        public:
            static const double DeltaTime();
            static const uint64_t FrameCount();
            static const double CurrentTime();

        private:
            static void SetDeltaTime(double ts);
            static void UpdateCurrentTime();
            static void IncremenetFrameCount(uint64_t value = 1);
            friend class Application;
            friend class Game;
    };
}  // namespace FooGame
