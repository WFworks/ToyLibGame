#include "IMEUtil.h"

#if defined(_WIN32)
#include <windows.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")

#elif defined(__APPLE__)
#include <Carbon/Carbon.h>

#elif defined(__linux__)
#include <SDL.h>
#endif

void IMEUtil::SetIMEEnabled(void* nativeWindowHandle, bool enabled)
{
#if defined(_WIN32)
    HWND hwnd = static_cast<HWND>(nativeWindowHandle);
    HIMC hIMC = ImmGetContext(hwnd);
    if (hIMC)
    {
        ImmSetOpenStatus(hIMC, enabled ? TRUE : FALSE);
        ImmReleaseContext(hwnd, hIMC);
    }

#elif defined(__APPLE__)
    if (!enabled)
    {
        CFArrayRef sources = TISCreateInputSourceList(NULL, false);
        if (sources)
        {
            CFIndex count = CFArrayGetCount(sources);
            for (CFIndex i = 0; i < count; ++i)
            {
                TISInputSourceRef source = (TISInputSourceRef)CFArrayGetValueAtIndex(sources, i);
                CFStringRef sourceID = (CFStringRef)TISGetInputSourceProperty(source, kTISPropertyInputSourceID);
                if (!sourceID) continue;

                // US配列 or ABC（JIS）を使う
                if (CFStringCompare(sourceID, CFSTR("com.apple.keylayout.US"), 0) == kCFCompareEqualTo ||
                    CFStringCompare(sourceID, CFSTR("com.apple.keylayout.ABC"), 0) == kCFCompareEqualTo)
                {
                    TISSelectInputSource(source);
                    break;
                }
            }
            CFRelease(sources);
        }
    }

#elif defined(__linux__)
    if (enabled)
        SDL_StartTextInput();
    else
        SDL_StopTextInput();
#endif
}
