#ifndef NATIVEAPP_H
#define NATIVEAPP_H

#include <string>
#include "UI/InputState.h"

// The Native App API.
//
// Implement these functions and you've got a native app. These are called
// from the framework, which exposes the native JNI api which is a bit
// more complicated.

// The very first function to be called after NativeGetAppInfo. Even NativeMix is not called
// before this, although it may be called at any point in time afterwards (on any thread!)
// This functions must NOT call OpenGL. Main thread.
void NativeInit(int wd, int ht);

// Runs after NativeInit() at some point. May (and probably should) call OpenGL.
// Should not initialize anything screen-size-dependent - do that in NativeResized.
void NativeInitGraphics();

// Called ~sixty times a second, delivers the current input state.
// Main thread.
void NativeUpdate(UI::InputState &input);

// Delivers touch events "instantly", without waiting for the next frame so that NativeUpdate can deliver.
// Useful for triggering audio events, saving a few ms.
// If you don't care about touch latency, just do a no-op implementation of this.
// time is not yet implemented. finger can be from 0 to 7, inclusive.
void NativeTouch(const UI::TouchInput &touch);
bool NativeKey(const UI::KeyInput &key);

// Called when it's time to render. If the device can keep up, this
// will also be called sixty times per second. Main thread.
void NativeRender();

// Called when it's time to shutdown. After this has been called,
// no more calls to any other function will be made from the framework
// before process exit.
// The graphics context should still be active when calling this, as freeing
// of graphics resources happens here.
// Main thread.
void NativeShutdownGraphics();
void NativeShutdown();

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
