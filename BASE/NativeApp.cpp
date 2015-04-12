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
#include "UI/LogoScreen.h"
using UI::LogoScreen;
#include "GFX/GfxResourceHolder.h"
using GFX::gl_lost_manager_init;
#include "THIN3D/Thin3D.h"
using THIN3D::T3DCreateGLContext;
using THIN3D::Thin3DContext;
#include "GFX/Texture.h"
using GFX::Atlas;
using GFX::AtlasImage;

namespace GLOBAL
{
    shared_ptr<ScreenManager> _ScreenManager;
    ScreenManager &screenManager() { return *_ScreenManager; }
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
    shared_ptr<Thin3DContext> _Thin3D;
    Thin3DContext &thin3DContext() { return *_Thin3D; }
    shared_ptr<Atlas> _UIAtlas;
    Atlas &uiAtlas() { return *_UIAtlas; }
    shared_ptr<AtlasImage> _UIAtlasImage[34];
    shared_ptr<AtlasImage> *uiAtlasImage() { return _UIAtlasImage; }

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
    GLOBAL::_DPIScale = 1.0f;
    GLOBAL::_PixelInDPS = 1.0f;

    GLOBAL::screenManager().switchScreen(new LogoScreen());
}

void NativeInitGraphics()
{
    GLOBAL::_GLExtensions = make_shared<GLExtensions>();
    GLOBAL::_GLState = make_shared<OpenGLState>();
    GLOBAL::_DrawBuf2D = make_shared<DrawBuffer>();
    GLOBAL::_DrawBuf2DFront = make_shared<DrawBuffer>();
    GLOBAL::_UIState = make_shared<UIState>();
    GLOBAL::_Thin3D = shared_ptr<Thin3DContext>(T3DCreateGLContext());
    GLOBAL::_UIAtlas = make_shared<Atlas>();
    for (int index = 0; index < 34; index++)
        GLOBAL::_UIAtlasImage[index] = make_shared<AtlasImage>();

    GLOBAL::drawBuffer2D().setAtlas(&GLOBAL::uiAtlas());

    // We do this here, instead of in NativeInitGraphics, because the display may be reset.
    // When it's reset we don't want to forget all our managed things.
    gl_lost_manager_init();
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
