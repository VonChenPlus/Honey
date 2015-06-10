// NativeApp implementation for platforms that will use that framework, like:
// Windows, Linux.
//
// Native is a cross platform framework. It's not very mature and mostly
// just built according to the needs of my own apps.
//


// Background worker threads should be spawned in NativeInit and joined
// in NativeShutdown.

#include "NativeApp.h"

#include <QFile>
#include <locale.h>
#include <memory>
#include <assert.h>
using std::shared_ptr;
using std::make_shared;

#include "SMARTGRAPH/UI/ScreenManager.h"
using UI::ScreenManager;
#include "SMARTGRAPH/GFX/GLExtensions.h"
using GFX::GLExtensions;
#include "SMARTGRAPH/GFX/GLState.h"
using GFX::OpenGLState;
#include "SMARTGRAPH/UI/UI.h"
using UI::UIState;
#include "SMARTGRAPH/GFX/GfxResourceHolder.h"
using GFX::gl_lost_manager_init;
#include "SMARTGRAPH/THIN3D/Thin3D.h"
using THIN3D::T3DCreateGLContext;
using THIN3D::Thin3DContext;
using THIN3D::T3DClear;
using THIN3D::T3DViewport;

namespace GLOBAL
{
    shared_ptr<ScreenManager> _ScreenManager;
    ScreenManager &screenManager() { assert(_ScreenManager); return *_ScreenManager; }
    shared_ptr<GLExtensions> _GLExtensions;
    GLExtensions &glExtensions() { assert(_GLExtensions); return *_GLExtensions; }
    shared_ptr<OpenGLState> _GLState;
    OpenGLState &glState() { assert(_GLState); return *_GLState; }
    std::string _sGLExtensions;
    std::string &sGlExtensions() { return _sGLExtensions;  }
    std::string _sEGLExtensions;
    std::string &sEGlExtensions() { return _sEGLExtensions;  }
    // This needs to be extern so that additional UI controls can be developed outside this file.
    shared_ptr<UIState> _UIState;
    UIState &uiState() { assert(_UIState); return *_UIState; }
    shared_ptr<UIState> _UIStatesaved;
    UIState &uiStateSaved() { assert(_UIStatesaved); return *_UIStatesaved; }
    shared_ptr<Thin3DContext> _Thin3D;
    Thin3DContext &thin3DContext() { assert(_Thin3D); return *_Thin3D; }

    int _DPXRes;
    int &dpXRes() { return _DPXRes; }
    int _DPYRes;
    int &dpYRes() { return _DPYRes; }
    int _PixelXRes;
    int &pixelXRes() { return _PixelXRes; }
    int _PixelYRes;
    int &pixelYRes() { return _PixelYRes; }
    int _DPI;
    int &dpi() { return _DPI; }
    float _DPIScale = 1.0f;
    float &dpiScale() { return _DPIScale; }
    float _PixelInDPS = 1.0f;
    float &pixelInDPS() { return _PixelInDPS; }
}

static recursive_mutex pendingMutex;
struct PendingMessage {
    std::string msg;
    std::string value;
};
static std::vector<PendingMessage> pendingMessages;

float CalculateDPIScale()
{
    // Sane default rather than check DPI
#ifdef __SYMBIAN32__
    return 1.4f;
#elif defined(USING_GLES2)
    return 1.2f;
#else
    return 1.0f;
#endif
}

void NativeInit(int wd, int ht)
{
    GLOBAL::pixelXRes() = wd;
    GLOBAL::pixelYRes() = ht;
    GLOBAL::dpiScale() = CalculateDPIScale();
    GLOBAL::dpXRes() = GLOBAL::pixelXRes() * GLOBAL::dpiScale();
    GLOBAL::dpYRes() = GLOBAL::pixelYRes() * GLOBAL::dpiScale();

    GLOBAL::_ScreenManager = make_shared<ScreenManager>();
    GLOBAL::_DPIScale = 1.0f;
    GLOBAL::_PixelInDPS = 1.0f;
}

void NativeInitGraphics()
{
    GLOBAL::_GLExtensions = make_shared<GLExtensions>();
    GLOBAL::_GLState = make_shared<OpenGLState>();
    GLOBAL::_UIState = make_shared<UIState>();
    GLOBAL::_UIStatesaved = make_shared<UIState>();
    GLOBAL::_Thin3D = shared_ptr<Thin3DContext>(T3DCreateGLContext());
    GLOBAL::screenManager().setThin3DContext(&GLOBAL::thin3DContext());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // We do this here, instead of in NativeInitGraphics, because the display may be reset.
    // When it's reset we don't want to forget all our managed things.
    gl_lost_manager_init();
}

void NativeUpdate(UI::InputState &input) {
    {
        lock_guard lock(pendingMutex);
        for (size_t i = 0; i < pendingMessages.size(); i++) {
            GLOBAL::screenManager().sendMessage(pendingMessages[i].msg.c_str(), pendingMessages[i].value.c_str());
        }
        pendingMessages.clear();
    }

    GLOBAL::screenManager().update(input);
}

void NativeTouch(const UI::TouchInput &touch) {
    GLOBAL::screenManager().touch(touch);
}

bool NativeKey(const UI::KeyInput &key) {
    return GLOBAL::screenManager().key(key);
}

void NativeRender() {
    GLOBAL::thin3DContext().clear(T3DClear::COLOR | T3DClear::DEPTH | T3DClear::STENCIL, 0xFF000000, 0.0f, 0);

    T3DViewport viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = GLOBAL::pixelXRes();
    viewport.Height = GLOBAL::pixelYRes();
    viewport.MaxDepth = 1.0;
    viewport.MinDepth = 0.0;
    GLOBAL::thin3DContext().setViewports(1, &viewport);

    GLOBAL::glState().depthWrite.set(GL_TRUE);
    GLOBAL::glState().colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    GLOBAL::glState().restore();

    GLOBAL::thin3DContext().setTargetSize(GLOBAL::pixelXRes(), GLOBAL::pixelYRes());

    GLOBAL::screenManager().render();
    if (GLOBAL::screenManager().getUIContext()->text()) {
        GLOBAL::screenManager().getUIContext()->text()->oncePerFrame();
    }

    GLOBAL::thin3DContext().setScissorEnabled(false);
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
