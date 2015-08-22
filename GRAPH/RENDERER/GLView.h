#ifndef GLVIEW_H
#define GLVIEW_H

#include <vector>
#include "GRAPH/BASE/Event.h"
#include "MATH/Size.h"
#include "MATH/Rectangle.h"
#include "EXTERNALS/glew/GL/glew.h"
#include "EXTERNALS/glfw/include/glfw3.h"
#ifdef _WIN32
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#ifndef GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WGL
#endif
#else
#ifndef GLFW_EXPOSE_NATIVE_NSGL
#define GLFW_EXPOSE_NATIVE_NSGL
#endif
#ifndef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#endif
#include "EXTERNALS/glfw/include/glfw3native.h"

namespace GRAPH
{
    /** There are some Resolution Policy for Adapt to the screen. */
    enum class ResolutionPolicy
    {
        /** The entire application is visible in the specified area without trying to preserve the original aspect ratio.
         * Distortion can occur, and the application may appear stretched or compressed.
         */
        EXACT_FIT,
        /** The entire application fills the specified area, without distortion but possibly with some cropping,
         * while maintaining the original aspect ratio of the application.
         */
        NO_BORDER,
        /** The entire application is visible in the specified area without distortion while maintaining the original
         * aspect ratio of the application. Borders can appear on two sides of the application.
         */
        SHOW_ALL,
        /** The application takes the height of the design resolution size and modifies the width of the internal
         * canvas so that it fits the aspect ratio of the device.
         * No distortion will occur however you must make sure your application works on different
         * aspect ratios.
         */
        FIXED_HEIGHT,
        /** The application takes the width of the design resolution size and modifies the height of the internal
         * canvas so that it fits the aspect ratio of the device.
         * No distortion will occur however you must make sure your application works on different
         * aspect ratios.
         */
        FIXED_WIDTH,

        UNKNOWN,
    };


    struct GLContextAttrs
    {
        int redBits;
        int greenBits;
        int blueBits;
        int alphaBits;
        int depthBits;
        int stencilBits;
    };

    class GLView : public HObject
    {
    public:
        GLView();
        virtual ~GLView();

        virtual void end() = 0;

        virtual bool isOpenGLReady() = 0;

        virtual void swapBuffers() = 0;

        virtual bool windowShouldClose() { return false; }

        static void setGLContextAttrs(GLContextAttrs& glContextAttrs);
        static GLContextAttrs getGLContextAttrs();

        static GLContextAttrs _glContextAttrs;

        virtual void pollEvents();

        virtual const MATH::Sizef& getFrameSize() const;
        virtual void setFrameSize(float width, float height);

        virtual void setFrameZoomFactor(float) {}
        virtual float getFrameZoomFactor() const { return 1.0; }

        virtual void setCursorVisible(bool) {}

        virtual int getRetinaFactor() const { return 1; }

        virtual bool setContentScaleFactor(float) { return false; }
        virtual float getContentScaleFactor() const { return 1.0; }

        virtual bool isRetinaDisplay() const { return false; }

        virtual MATH::Sizef getVisibleSize() const;
        virtual MATH::Vector2f getVisibleOrigin() const;
        virtual MATH::Rectf getVisibleRect() const;

        virtual void setDesignResolutionSize(float width, float height, ResolutionPolicy resolutionPolicy);
        virtual const MATH::Sizef&  getDesignResolutionSize() const;

        virtual void setViewPortInPoints(float x , float y , float w , float h);
        virtual void setScissorInPoints(float x , float y , float w , float h);

        virtual bool isScissorEnabled();
        virtual MATH::Rectf getScissorRect() const;

        virtual void setViewName(const std::string& viewname);
        const std::string& getViewName() const;

        virtual void handleTouchesBegin(int num, intptr_t ids[], float xs[], float ys[]);
        virtual void handleTouchesMove(int num, intptr_t ids[], float xs[], float ys[]);
        virtual void handleTouchesEnd(int num, intptr_t ids[], float xs[], float ys[]);
        virtual void handleTouchesCancel(int num, intptr_t ids[], float xs[], float ys[]);

        const MATH::Rectf& getViewPortRect() const;

        std::vector<Touch*> getAllTouches() const;

        float getScaleX() const;
        float getScaleY() const;

        ResolutionPolicy getResolutionPolicy() const { return _resolutionPolicy; }

    protected:
        void updateDesignResolutionSize();

        void handleTouchesOfEndOrCancel(EventTouch::EventCode eventCode, int num, intptr_t ids[], float xs[], float ys[]);

        // real screen size
        MATH::Sizef _screenSize;
        // resolution size, it is the size appropriate for the app resources.
        MATH::Sizef _designResolutionSize;
        // the view port size
        MATH::Rectf _viewPortRect;
        // the view name
        std::string _viewName;

        float _scaleX;
        float _scaleY;
        ResolutionPolicy _resolutionPolicy;
    };

    class GLViewImpl : public GLView
    {
    public:
        static GLViewImpl* create(const std::string& viewName);
        static GLViewImpl* createWithRect(const std::string& viewName, MATH::Rectf size, float frameZoomFactor = 1.0f);
        static GLViewImpl* createWithFullScreen(const std::string& viewName);
        static GLViewImpl* createWithFullScreen(const std::string& viewName, const GLFWvidmode &videoMode, GLFWmonitor *monitor);

        float getFrameZoomFactor() const override;

        virtual void setViewPortInPoints(float x , float y , float w , float h) override;
        virtual void setScissorInPoints(float x , float y , float w , float h) override;


        bool windowShouldClose() override;
        void pollEvents() override;
        GLFWwindow* getWindow() const { return _mainWindow; }

        virtual bool isOpenGLReady() override;
        virtual void end() override;
        virtual void swapBuffers() override;
        virtual void setFrameSize(float width, float height) override;

        void setFrameZoomFactor(float zoomFactor) override;

        virtual void setCursorVisible(bool isVisible) override;

        int getRetinaFactor() const override { return _retinaFactor; }

    #ifdef _WIN32
        HWND getWin32Window() { return glfwGetWin32Window(_mainWindow); }
    #else
        id getCocoaWindow() { return glfwGetCocoaWindow(_mainWindow); }
    #endif

    protected:
        GLViewImpl();
        virtual ~GLViewImpl();

        bool initWithRect(const std::string& viewName, MATH::Rectf rect, float frameZoomFactor);
        bool initWithFullScreen(const std::string& viewName);
        bool initWithFullscreen(const std::string& viewname, const GLFWvidmode &videoMode, GLFWmonitor *monitor);

        void initGlew();

        void updateFrameSize();

        // GLFW callbacks
        void onGLFWError(int errorID, const char* errorDesc);
        void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify);
        void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y);
        void onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y);
        void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        void onGLFWCharCallback(GLFWwindow* window, unsigned int character);
        void onGLFWWindowPosCallback(GLFWwindow* windows, int x, int y);
        void onGLFWframebuffersize(GLFWwindow* window, int w, int h);
        void onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height);

        bool _captured;
        bool _supportTouch;
        bool _isInRetinaMonitor;
        bool _isRetinaEnabled;
        int  _retinaFactor;  // Should be 1 or 2

        float _frameZoomFactor;

        GLFWwindow* _mainWindow;
        GLFWmonitor* _monitor;

        float _mouseX;
        float _mouseY;

        friend class GLFWEventHandler;

    private:
        DISALLOW_COPY_AND_ASSIGN(GLViewImpl)
    };
}

#endif // GLVIEW_H
