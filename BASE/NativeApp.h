#ifndef NATIVEAPP_H
#define NATIVEAPP_H

#include <string>

// The Native App API.
//
// Implement these functions and you've got a native app. These are called
// from the framework, which exposes the native JNI api which is a bit
// more complicated.

// The very first function to be called after NativeGetAppInfo. Even NativeMix is not called
// before this, although it may be called at any point in time afterwards (on any thread!)
// This functions must NOT call OpenGL. Main thread.
void NativeInit();

enum SystemProperty
{
    SYSPROP_NAME,
    SYSPROP_LANGREGION,
    SYSPROP_CPUINFO,
    SYSPROP_CLIPBOARD_TEXT,
    SYSPROP_GPUDRIVER_VERSION,

    // Available as Int:
    SYSPROP_SYSTEMVERSION,
    SYSPROP_DISPLAY_XRES,
    SYSPROP_DISPLAY_YRES,
    SYSPROP_DISPLAY_REFRESH_RATE,  // returns 1000*the refresh rate in Hz as it can be non-integer
    SYSPROP_MOGA_VERSION,

    // Exposed on Android. Choosing the optimal sample rate for audio
    // will result in lower latencies. Buffer size is automatically matched
    // by the OpenSL audio backend, only exposed here for debugging/info.
    SYSPROP_AUDIO_SAMPLE_RATE,
    SYSPROP_AUDIO_FRAMES_PER_BUFFER,
    SYSPROP_AUDIO_OPTIMAL_SAMPLE_RATE,
    SYSPROP_AUDIO_OPTIMAL_FRAMES_PER_BUFFER,
};

std::string System_GetProperty(SystemProperty prop);
int System_GetPropertyInt(SystemProperty prop);

#endif // NATIVEAPP_H
