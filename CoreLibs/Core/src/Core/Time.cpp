#include "Time.h"
#include "../Core/Window.h"
namespace FooGame
{
    struct TimeData
    {
            double DeltaTime    = 0;
            uint64_t FrameCount = 0;
            double CurrentTime  = 0;
    } timeData;

    const double Time::DeltaTime()
    {
        return timeData.DeltaTime;
    }
    const uint64_t Time::FrameCount()
    {
        return timeData.FrameCount;
    }
    const double Time::CurrentTime()
    {
        return timeData.CurrentTime;
    }
    void Time::IncremenetFrameCount(uint64_t value)
    {
        timeData.FrameCount++;
    }
    void Time::SetDeltaTime(double ts)
    {
        timeData.DeltaTime = ts;
    }
    void Time::UpdateCurrentTime()
    {
        timeData.CurrentTime = Window::Get().GetTime();
    }

}  // namespace FooGame
