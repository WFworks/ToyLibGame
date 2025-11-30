#pragma once

class IMEUtil
{
public:
    // nativeWindowHandle は Windows のみ必要。他は nullptr でOK
    static void SetIMEEnabled(void* nativeWindowHandle, bool enabled);
};
