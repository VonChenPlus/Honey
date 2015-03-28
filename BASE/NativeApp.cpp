// NativeApp implementation for platforms that will use that framework, like:
// Windows, Linux.
//
// Native is a cross platform framework. It's not very mature and mostly
// just built according to the needs of my own apps.
//


// Background worker threads should be spawned in NativeInit and joined
// in NativeShutdown.

#include <locale.h>

#include "UI/ScreenManager.h"

using UI::ScreenManager;

ScreenManager *screenManager;

void NativeInit() 
{
    setlocale( LC_ALL, "C" );
    screenManager = new ScreenManager;
}
