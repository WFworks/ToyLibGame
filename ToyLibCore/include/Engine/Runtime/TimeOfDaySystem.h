#pragma once

struct GameTime
{
    int   day    = 1;   // 日付を扱いたくなったとき用（とりあえず1日目）
    int   hour   = 6;   // 0-23
    int   minute = 0;   // 0-59
    float second = 0.0f;

    float GetTimeOfDay01() const
    {
        float totalSec = hour * 3600.0f + minute * 60.0f + second;
        return totalSec / (24.0f * 3600.0f);
    }

    float GetHourFloat() const
    {
        return hour + minute / 60.0f + second / 3600.0f;
    }
};

class TimeOfDaySystem
{
public:
    TimeOfDaySystem();
    
    void Update(float deltaTime);

    void SetTime(int hour, int minute = 0, float second = 0.0f);

    void SetTimeScale(float scale) { mTimeScale = scale; } // 0で停止
    void SetRunning(bool r) { mRunning = r; }

    const GameTime& GetGameTime()  const { return mTime; }
    float GetTimeOfDay01() const { return mTime.GetTimeOfDay01(); }
    float GetHourFloat() const { return mTime.GetHourFloat(); }
    int GetHour() const { return mTime.hour; }
    int GetMinute() const { return mTime.minute; }
    int GetSccond() const { return mTime.second; }

private:
    GameTime mTime;
    float mTimeScale; // 1秒で1分進む、みたいな
    bool mRunning;
};
