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
#include "GFX/GLExtensions.h"
using GFX::GLExtensions;
#include "GFX/GLState.h"
using GFX::OpenGLState;
#include "GFX/DrawBuffer.h"
using GFX::DrawBuffer;
#include "UI/UI.h"
using UI::UIState;

namespace GLOBAL
{
    shared_ptr<ScreenManager> _ScreenManager;
    const ScreenManager &screenManager() { return *_ScreenManager; }
    shared_ptr<GLExtensions> _GLExtensions;
    GLExtensions &glExtensions() { return *_GLExtensions; }
    shared_ptr<OpenGLState> _GLState;
    OpenGLState &glState() { return *_GLState; }
    std::string _sGLExtensions;
    std::string _sEGLExtensions;
    // This is the drawbuffer used for UI. Remember to flush it at the end of the frame.
    // TODO: One should probably pass it in through UIInit.
    shared_ptr<DrawBuffer> _DrawBuf2D;
    DrawBuffer &drawBuffer2D() { return *_DrawBuf2D; }
    shared_ptr<DrawBuffer> _DrawBuf2DFront;	// for things that need to be on top of the rest
    DrawBuffer &drawBuffer2DFront() { return *_DrawBuf2DFront; }
    // This needs to be extern so that additional UI controls can be developed outside this file.
    shared_ptr<UIState> _UIState;
    UIState &uiState() { return *_UIState; }
    shared_ptr<UIState> _UIStatesaved;
    UIState &uiStateSaved() { return *_UIStatesaved; }

    int _DPXRes;
    int dpXRes() { return _DPXRes; }
    int _DPYRes;
    int dpYRes() { return _DPYRes; }
    float _DPIScale = 1.0f;
    float dpiScale() { return _DPIScale; }
    float _PixelInDPS = 1.0f;
    float pixelInDPS() { return _PixelInDPS; }
}

void NativeInit() 
{
    GLOBAL::_ScreenManager = make_shared<ScreenManager>();
    GLOBAL::_GLExtensions = make_shared<GLExtensions>();
    GLOBAL::_GLState = make_shared<OpenGLState>();
    GLOBAL::_DrawBuf2D = make_shared<DrawBuffer>();
    GLOBAL::_DrawBuf2DFront = make_shared<DrawBuffer>();
    GLOBAL::_UIState = make_shared<UIState>();
    GLOBAL::_DPIScale = 1.0f;
    GLOBAL::_PixelInDPS = 1.0f;
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
