// NativeApp implementation for platforms that will use that framework, like:
// Windows, Linux.
//
// Native is a cross platform framework. It's not very mature and mostly
// just built according to the needs of my own apps.
//


// Background worker threads should be spawned in NativeInit and joined
// in NativeShutdown.

#include "NativeApp.h"

#include <locale.h>
#include <memory>
using std::shared_ptr;
using std::make_shared;

#include "UI/ScreenManager.h"
using UI::ScreenManager;

shared_ptr<ScreenManager> screenManager;

void NativeInit() 
{
    setlocale( LC_ALL, "C" );
    screenManager = make_shared<ScreenManager>();
}

std::string System_GetProperty(SystemProperty prop)
{
    switch (prop)
    {
    case SYSPROP_NAME:
#ifdef Q_OS_LINUX
        return "Qt:Linux";
#elif defined(_WIN32)
        return "Qt:Windows";
#else
        return "Qt";
#endif
    default:
        return "";
    }
}

int System_GetPropertyInt(SystemProperty prop)
{
  switch (prop)
  {
  case SYSPROP_AUDIO_SAMPLE_RATE:
    return 44100;
    case SYSPROP_DISPLAY_REFRESH_RATE:
        return 60000;
  default:
    return -1;
  }
}
