#include "Engine/Runtime/TimeOfDaySystem.h"
#include <algorithm>
#include <iostream>

TimeOfDaySystem::TimeOfDaySystem()
: mTimeScale(60.f) // 60.f　1秒で1分進む
, mRunning(true)
{
    
}

void TimeOfDaySystem::SetTime(int hour, int minute, float second)
{
    mTime.hour   = std::clamp(hour,   0, 23);
    mTime.minute = std::clamp(minute, 0, 59);
    mTime.second = std::clamp(second, 0.0f, 59.999f);
}


void TimeOfDaySystem::Update(float deltaTime)
{
    if (!mRunning) return;

    float scaled = deltaTime * mTimeScale; // 倍速/スロー
    mTime.second += scaled;

    while (mTime.second >= 60.0f)
    {
        mTime.second -= 60.0f;
        ++mTime.minute;
    }
    while (mTime.minute >= 60)
    {
        mTime.minute -= 60;
        ++mTime.hour;
    }
    while (mTime.hour >= 24)
    {
        mTime.hour -= 24;
        ++mTime.day;
    }
}
