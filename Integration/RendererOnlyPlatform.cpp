/*
 * Small platform hooks needed when The-Forge is embedded without its
 * application layer.
 */

#include "OS/Interfaces/IOperatingSystem.h"

// The application/window layer normally owns this callback. The embedded
// renderer has no reset coordinator, so keep device-loss handling harmless.
#if (defined(_WINDOWS) || defined(__ANDROID__)) && !defined(THE_FORGE_RENDERER_RESET_HANDLER)
extern "C" void requestReset(const ResetDesc* pResetDesc)
{
    (void)pResetDesc;
}
#endif

#if defined(ANDROID)
extern "C" jint AndroidAttachToCurrentThread(WindowDesc* pWindow, JNIEnv** ppEnv)
{
    ASSERT(pWindow);
    ASSERT(pWindow->handle.activity);
    ASSERT(pWindow->handle.activity->vm);
    return pWindow->handle.activity->vm->AttachCurrentThread(ppEnv, NULL);
}
#endif

#if defined(__APPLE__)
#include "Utilities/Interfaces/ITime.h"

#include <time.h>

extern "C" uint32_t getSystemTime(void)
{
    struct timespec timeValue = {};
    clock_gettime(CLOCK_MONOTONIC, &timeValue);
    return (uint32_t)(timeValue.tv_sec * 1000 + timeValue.tv_nsec / 1000000);
}

extern "C" int64_t getUSec(bool precise)
{
    (void)precise;
    struct timespec timeValue = {};
    clock_gettime(CLOCK_MONOTONIC, &timeValue);
    return (int64_t)timeValue.tv_sec * 1000000 + timeValue.tv_nsec / 1000;
}

extern "C" int64_t getTimerFrequency(void)
{
    return 1000000;
}

extern "C" uint32_t getTimeSinceStart(void)
{
    return (uint32_t)time(NULL);
}
#endif
