#include "SingleInstance.h"
#include <iostream>
#ifdef _WIN32
    // Windows用は特に追加不要
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    #include <cstring>
#endif

const char* LOCK_FILE_NAME = "/tmp/my_program.lock";

SingleInstance::SingleInstance()
: mIsLocked(false)
{
#ifdef _WIN32
    mMutex = CreateMutexA(NULL, FALSE, "Global\\ToyLib_Sample_SingleInstance_Mutex");
    if (mMutex == NULL)
    {
        std::cerr << "CreateMutex failed: " << GetLastError() << std::endl;
        return;
    }
    DWORD err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS)
    {
        std::cerr << "プログラムはすでに起動しています。GetLastError: " << err << std::endl;
        CloseHandle(mMutex);
        mMutex = NULL;
        return;
    }
#else
    mFd = open(LOCK_FILE_NAME, O_CREAT | O_RDWR, 0666);
    if (mFd == -1)
    {
        perror("ロックファイルを開けませんでした");
        return;
    }

    if (lockf(mFd, F_TLOCK, 0) == -1)
    {
        std::cerr << "プログラムはすでに起動しています: " << strerror(errno) << std::endl;
        close(mFd);
        return;
    }
#endif
    mIsLocked = true;
}

SingleInstance::~SingleInstance()
{
    if (mIsLocked)
    {
#ifdef _WIN32
        if (mMutex)
        {
            CloseHandle(mMutex);
        }
#else
        if (mFd != -1)
        {
            close(mFd);
            unlink(LOCK_FILE_NAME); // ロックファイルを削除
        }
#endif
    }
}

