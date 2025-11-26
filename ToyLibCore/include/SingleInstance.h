#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

class SingleInstance
{
private:
#ifdef _WIN32
    HANDLE mMutex;
#else
    int mFd;
#endif
    bool mIsLocked;

public:
    SingleInstance();

    ~SingleInstance();

    bool IsLocked() const { return mIsLocked; }
};

