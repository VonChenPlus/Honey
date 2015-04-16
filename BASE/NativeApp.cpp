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
using THIN3D::T3DViewport;
using THIN3D::T3DClear;
using THIN3D::T3DImageType;
using THIN3D::T3DShaderSetPreset;
using THIN3D::Thin3DTexture;
#include "GFX/Texture.h"
using GFX::Atlas;
using GFX::AtlasImage;
using UI::Theme;
#include "UI/UIContext.h"
using UI::UIContext;
#include "MATH/Bounds.h"
using MATH::Bounds;
#include "MATH/Matrix.h"
using MATH::Matrix4x4;

namespace GLOBAL
{
    shared_ptr<ScreenManager> _ScreenManager;
    ScreenManager &screenManager() { assert(_ScreenManager); return *_ScreenManager; }
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
    shared_ptr<AtlasImage> _UIAtlasImage[34];
    Atlas &uiAtlas() { return *_UIAtlas; }
    shared_ptr<AtlasImage> *uiAtlasImage() { return _UIAtlasImage; }
    shared_ptr<Theme> _UITheme;
    Theme &uiTheme() { return *_UITheme; }
    shared_ptr<UIContext> _UIContext;
    UIContext &uiContext() { return *_UIContext; }
    shared_ptr<Thin3DTexture> _UITexture;
    Thin3DTexture &uiTexture() { return *_UITexture; }

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

static bool resized = false;
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

    GLOBAL::_GLExtensions = make_shared<GLExtensions>();
    GLOBAL::_GLState = make_shared<OpenGLState>();
    GLOBAL::_UIState = make_shared<UIState>();
    GLOBAL::_UIStatesaved = make_shared<UIState>();
    GLOBAL::_UIAtlas = make_shared<Atlas>();
    for (int index = 0; index < 34; index++)
        GLOBAL::_UIAtlasImage[index] = make_shared<AtlasImage>();
    GLOBAL::_UITheme = make_shared<Theme>();
    GLOBAL::_DrawBuf2D = make_shared<DrawBuffer>();
    GLOBAL::_DrawBuf2DFront = make_shared<DrawBuffer>();

    GLOBAL::screenManager().switchScreen(new LogoScreen());
}

void NativeInitGraphics()
{
    // init
    GLOBAL::_Thin3D = shared_ptr<Thin3DContext>(T3DCreateGLContext());

    GLOBAL::drawBuffer2D().setAtlas(&GLOBAL::uiAtlas());
    GLOBAL::drawBuffer2DFront().setAtlas(&GLOBAL::uiAtlas());
    GLOBAL::drawBuffer2D().init(&GLOBAL::thin3DContext());
    GLOBAL::drawBuffer2DFront().init(&GLOBAL::thin3DContext());

    QFile asset("S:\\OpenSource\\Hive\\Native\\ASSERT\\ui_atlas_lowmem.zim");
    asset.open(QIODevice::ReadOnly);
    uint8_t *contents = new uint8_t[asset.size()+1];
    memcpy(contents, (uint8_t*)asset.readAll().data(), asset.size());
    contents[asset.size()] = 0;
    GLOBAL::_UITexture = shared_ptr<Thin3DTexture>(GLOBAL::thin3DContext().createTextureFromFileData(contents, asset.size(), T3DImageType::ZIM));
    delete[] contents;
    asset.close();

    GLOBAL::_UIContext = make_shared<UIContext>();
    GLOBAL::uiContext().theme = &GLOBAL::uiTheme();
    GLOBAL::uiContext().init(&GLOBAL::thin3DContext(), 
        GLOBAL::thin3DContext().getShaderSetPreset(T3DShaderSetPreset::SS_TEXTURE_COLOR_2D),
        GLOBAL::thin3DContext().getShaderSetPreset(T3DShaderSetPreset::SS_COLOR_2D), 
        &GLOBAL::uiTexture(), &GLOBAL::drawBuffer2D(), &GLOBAL::drawBuffer2DFront());
    if (GLOBAL::uiContext().text())
        GLOBAL::uiContext().text()->setFont("Tahoma", 20, 0);

    GLOBAL::screenManager().setUIContext(&GLOBAL::uiContext());
    GLOBAL::screenManager().setThin3DContext(&GLOBAL::thin3DContext());

    GLOBAL::uiTheme().checkOn = I_CHECKEDBOX;
    GLOBAL::uiTheme().checkOff = I_SQUARE;
    GLOBAL::uiTheme().whiteImage = I_SOLIDWHITE;
    GLOBAL::uiTheme().sliderKnob = I_CIRCLE;
    GLOBAL::uiTheme().dropShadow4Grid = I_DROP_SHADOW;

    GLOBAL::uiTheme().itemStyle.background = UI::Drawable(0x55000000);
    GLOBAL::uiTheme().itemStyle.fgColor = 0xFFFFFFFF;
    GLOBAL::uiTheme().itemFocusedStyle.background = UI::Drawable(0xFFedc24c);
    GLOBAL::uiTheme().itemDownStyle.background = UI::Drawable(0xFFbd9939);
    GLOBAL::uiTheme().itemDownStyle.fgColor = 0xFFFFFFFF;
    GLOBAL::uiTheme().itemDisabledStyle.background = UI::Drawable(0x55E0D4AF);
    GLOBAL::uiTheme().itemDisabledStyle.fgColor = 0x80EEEEEE;
    GLOBAL::uiTheme().itemHighlightedStyle.background = UI::Drawable(0x55bdBB39);
    GLOBAL::uiTheme().itemHighlightedStyle.fgColor = 0xFFFFFFFF;

    GLOBAL::uiTheme().buttonStyle = GLOBAL::uiTheme().itemStyle;
    GLOBAL::uiTheme().buttonFocusedStyle = GLOBAL::uiTheme().itemFocusedStyle;
    GLOBAL::uiTheme().buttonDownStyle = GLOBAL::uiTheme().itemDownStyle;
    GLOBAL::uiTheme().buttonDisabledStyle = GLOBAL::uiTheme().itemDisabledStyle;
    GLOBAL::uiTheme().buttonHighlightedStyle = GLOBAL::uiTheme().itemHighlightedStyle;

    GLOBAL::uiTheme().popupTitle.fgColor = 0xFFE3BE59;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // We do this here, instead of in NativeInitGraphics, because the display may be reset.
    // When it's reset we don't want to forget all our managed things.
    gl_lost_manager_init();
}

void NativeResized()
{
    // Modifying the bounds here can be used to "inset" the whole image to gain borders for TV overscan etc.
    // The UI now supports any offset but not the EmuScreen yet.
    if (GLOBAL::_UIContext)
        GLOBAL::uiContext().setBounds(Bounds(0, 0, GLOBAL::dpXRes(), GLOBAL::dpYRes()));
}

void NativeUpdate(_INPUT::InputState &input) {
    {
        lock_guard lock(pendingMutex);
        for (size_t i = 0; i < pendingMessages.size(); i++) {
            GLOBAL::screenManager().sendMessage(pendingMessages[i].msg.c_str(), pendingMessages[i].value.c_str());
        }
        pendingMessages.clear();
    }

    GLOBAL::screenManager().update(input);
}

void NativeTouch(const _INPUT::TouchInput &touch) {
    GLOBAL::screenManager().touch(touch);
}

bool NativeKey(const _INPUT::KeyInput &key) {
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

    float xres = GLOBAL::dpXRes();
    float yres = GLOBAL::dpYRes();

    // Apply the UIContext bounds as a 2D transformation matrix.
    Matrix4x4 ortho;
    ortho.setOrtho(0.0f, xres, yres, 0.0f, -1.0f, 1.0f);

    GLOBAL::drawBuffer2D().setDrawMatrix(ortho);
    GLOBAL::drawBuffer2DFront().setDrawMatrix(ortho);

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
