#include <map>
#include "GRAPH/BASE/Event.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/RENDERER/GLCommon.h"
#include "GRAPH/RENDERER/GLView.h"
#include "MATH/MathDef.h"
#include "GRAPH/RENDERER/FrameBuffer.h"
#include "GRAPH/BASE/Camera.h"
#include "GRAPH/BASE/EventDispatcher.h"
#include "UTILS/STRING/StringUtils.h"
#include "UTILS/STRING/UTFUtils.h"
#include "GRAPH/BASE/IMEDispatcher.h"

namespace GRAPH
{
    static Touch* g_touches[EventTouch::MAX_TOUCHES] = { nullptr };
    static unsigned int g_indexBitsUsed = 0;
    // System touch pointer ID (It may not be ascending order number) <-> Ascending order number from 0
    static std::map<intptr_t, int> g_touchIdReorderMap;

    static int getUnUsedIndex()
    {
        int i;
        int temp = g_indexBitsUsed;

        for (i = 0; i < EventTouch::MAX_TOUCHES; i++) {
            if (! (temp & 0x00000001)) {
                g_indexBitsUsed |= (1 <<  i);
                return i;
            }

            temp >>= 1;
        }

        // all bits are used
        return -1;
    }

    static std::vector<Touch*> getAllTouchesVector()
    {
        std::vector<Touch*> ret;
        int i;
        int temp = g_indexBitsUsed;

        for (i = 0; i < EventTouch::MAX_TOUCHES; i++) {
            if ( temp & 0x00000001) {
                ret.push_back(g_touches[i]);
            }
            temp >>= 1;
        }
        return ret;
    }

    static void removeUsedIndexBit(int index)
    {
        if (index < 0 || index >= EventTouch::MAX_TOUCHES)
        {
            return;
        }

        unsigned int temp = 1 << index;
        temp = ~temp;
        g_indexBitsUsed &= temp;
    }

    //default context attributions are setted as follows
    GLContextAttrs GLView::_glContextAttrs = {5, 6, 5, 0, 16, 0};

    void GLView::setGLContextAttrs(GLContextAttrs& glContextAttrs)
    {
        _glContextAttrs = glContextAttrs;
    }

    GLContextAttrs GLView::getGLContextAttrs()
    {
    return _glContextAttrs;
    }

    GLView::GLView()
    : _scaleX(1.0f)
    , _scaleY(1.0f)
    , _resolutionPolicy(ResolutionPolicy::UNKNOWN)
    {
    }

    GLView::~GLView()
    {

    }

    void GLView::pollEvents()
    {
    }

    void GLView::updateDesignResolutionSize()
    {
        if (_screenSize.width > 0 && _screenSize.height > 0
            && _designResolutionSize.width > 0 && _designResolutionSize.height > 0)
        {
            _scaleX = (float)_screenSize.width / _designResolutionSize.width;
            _scaleY = (float)_screenSize.height / _designResolutionSize.height;

            if (_resolutionPolicy == ResolutionPolicy::NO_BORDER)
            {
                _scaleX = _scaleY = MATH::MATH_MAX(_scaleX, _scaleY);
            }

            else if (_resolutionPolicy == ResolutionPolicy::SHOW_ALL)
            {
                _scaleX = _scaleY = MATH::MATH_MIN(_scaleX, _scaleY);
            }

            else if ( _resolutionPolicy == ResolutionPolicy::FIXED_HEIGHT) {
                _scaleX = _scaleY;
                _designResolutionSize.width = ceilf(_screenSize.width/_scaleX);
            }

            else if ( _resolutionPolicy == ResolutionPolicy::FIXED_WIDTH) {
                _scaleY = _scaleX;
                _designResolutionSize.height = ceilf(_screenSize.height/_scaleY);
            }

            // calculate the rect of viewport
            float viewPortW = _designResolutionSize.width * _scaleX;
            float viewPortH = _designResolutionSize.height * _scaleY;

            _viewPortRect.setRect((_screenSize.width - viewPortW) / 2, (_screenSize.height - viewPortH) / 2, viewPortW, viewPortH);

            // reset director's member variables to fit visible rect
            auto director = Director::getInstance();
            director->_winSizeInPoints = getDesignResolutionSize();
            director->_isStatusLabelUpdated = true;
            director->setGLDefaultValues();
        }
    }

    void GLView::setDesignResolutionSize(float width, float height, ResolutionPolicy resolutionPolicy)
    {
        if (width == 0.0f || height == 0.0f)
        {
            return;
        }

        _designResolutionSize.setSize(width, height);
        _resolutionPolicy = resolutionPolicy;

        updateDesignResolutionSize();
    }

    const MATH::Sizef& GLView::getDesignResolutionSize() const
    {
        return _designResolutionSize;
    }

    const MATH::Sizef& GLView::getFrameSize() const
    {
        return _screenSize;
    }

    void GLView::setFrameSize(float width, float height)
    {
        _designResolutionSize = _screenSize = MATH::Sizef(width, height);
    }

    MATH::Rectf GLView::getVisibleRect() const
    {
        MATH::Rectf ret;
        ret.size = getVisibleSize();
        ret.origin = getVisibleOrigin();
        return ret;
    }

    MATH::Sizef GLView::getVisibleSize() const
    {
        if (_resolutionPolicy == ResolutionPolicy::NO_BORDER)
        {
            return MATH::Sizef(_screenSize.width/_scaleX, _screenSize.height/_scaleY);
        }
        else
        {
            return _designResolutionSize;
        }
    }

    MATH::Vector2f GLView::getVisibleOrigin() const
    {
        if (_resolutionPolicy == ResolutionPolicy::NO_BORDER)
        {
            return MATH::Vector2f((_designResolutionSize.width - _screenSize.width/_scaleX)/2,
                               (_designResolutionSize.height - _screenSize.height/_scaleY)/2);
        }
        else
        {
            return MATH::Vec2fZERO;
        }
    }

    void GLView::setViewPortInPoints(float x , float y , float w , float h)
    {
        Viewport vp = {(float)(x * _scaleX + _viewPortRect.origin.x),
            (float)(y * _scaleY + _viewPortRect.origin.y),
            (float)(w * _scaleX),
            (float)(h * _scaleY)};
        Camera::setDefaultViewport(vp);
    }

    void GLView::setScissorInPoints(float x , float y , float w , float h)
    {
        glScissor((GLint)(x * _scaleX + _viewPortRect.origin.x),
                  (GLint)(y * _scaleY + _viewPortRect.origin.y),
                  (GLsizei)(w * _scaleX),
                  (GLsizei)(h * _scaleY));
    }

    bool GLView::isScissorEnabled()
    {
        return (GL_FALSE == glIsEnabled(GL_SCISSOR_TEST)) ? false : true;
    }

    MATH::Rectf GLView::getScissorRect() const
    {
        GLfloat params[4];
        glGetFloatv(GL_SCISSOR_BOX, params);
        float x = (params[0] - _viewPortRect.origin.x) / _scaleX;
        float y = (params[1] - _viewPortRect.origin.y) / _scaleY;
        float w = params[2] / _scaleX;
        float h = params[3] / _scaleY;
        return MATH::Rectf(x, y, w, h);
    }

    void GLView::setViewName(const std::string& viewname )
    {
        _viewName = viewname;
    }

    const std::string& GLView::getViewName() const
    {
        return _viewName;
    }

    void GLView::handleTouchesBegin(int num, intptr_t ids[], float xs[], float ys[])
    {
        intptr_t id = 0;
        float x = 0.0f;
        float y = 0.0f;
        int unusedIndex = 0;
        EventTouch touchEvent;

        for (int i = 0; i < num; ++i)
        {
            id = ids[i];
            x = xs[i];
            y = ys[i];

            auto iter = g_touchIdReorderMap.find(id);

            // it is a new touch
            if (iter == g_touchIdReorderMap.end())
            {
                unusedIndex = getUnUsedIndex();

                // The touches is more than MAX_TOUCHES ?
                if (unusedIndex == -1) {
                    continue;
                }

                Touch* touch = g_touches[unusedIndex] = new (std::nothrow) Touch();
                touch->setTouchInfo(unusedIndex, (x - _viewPortRect.origin.x) / _scaleX,
                                         (y - _viewPortRect.origin.y) / _scaleY);

                g_touchIdReorderMap.insert(std::make_pair(id, unusedIndex));
                touchEvent._touches.push_back(touch);
            }
        }

        if (touchEvent._touches.size() == 0)
        {
            return;
        }

        touchEvent._eventCode = EventTouch::EventCode::BEGAN;
        auto dispatcher = Director::getInstance()->getEventDispatcher();
        dispatcher->dispatchEvent(&touchEvent);
    }

    void GLView::handleTouchesMove(int num, intptr_t ids[], float xs[], float ys[])
    {
        intptr_t id = 0;
        float x = 0.0f;
        float y = 0.0f;
        EventTouch touchEvent;

        for (int i = 0; i < num; ++i)
        {
            id = ids[i];
            x = xs[i];
            y = ys[i];

            auto iter = g_touchIdReorderMap.find(id);
            if (iter == g_touchIdReorderMap.end())
            {
                continue;
            }

            Touch* touch = g_touches[iter->second];
            if (touch)
            {
                touch->setTouchInfo(iter->second, (x - _viewPortRect.origin.x) / _scaleX,
                                    (y - _viewPortRect.origin.y) / _scaleY);

                touchEvent._touches.push_back(touch);
            }
            else
            {
                return;
            }
        }

        if (touchEvent._touches.size() == 0)
        {
            return;
        }

        touchEvent._eventCode = EventTouch::EventCode::MOVED;
        auto dispatcher = Director::getInstance()->getEventDispatcher();
        dispatcher->dispatchEvent(&touchEvent);
    }

    void GLView::handleTouchesOfEndOrCancel(EventTouch::EventCode eventCode, int num, intptr_t ids[], float xs[], float ys[])
    {
        intptr_t id = 0;
        float x = 0.0f;
        float y = 0.0f;
        EventTouch touchEvent;

        for (int i = 0; i < num; ++i)
        {
            id = ids[i];
            x = xs[i];
            y = ys[i];

            auto iter = g_touchIdReorderMap.find(id);
            if (iter == g_touchIdReorderMap.end())
            {
                continue;
            }

            /* Add to the set to send to the director */
            Touch* touch = g_touches[iter->second];
            if (touch)
            {
                touch->setTouchInfo(iter->second, (x - _viewPortRect.origin.x) / _scaleX,
                                    (y - _viewPortRect.origin.y) / _scaleY);

                touchEvent._touches.push_back(touch);

                g_touches[iter->second] = nullptr;
                removeUsedIndexBit(iter->second);

                g_touchIdReorderMap.erase(id);
            }
            else
            {
                return;
            }

        }

        if (touchEvent._touches.size() == 0)
        {
            return;
        }

        touchEvent._eventCode = eventCode;
        auto dispatcher = Director::getInstance()->getEventDispatcher();
        dispatcher->dispatchEvent(&touchEvent);

        for (auto& touch : touchEvent._touches)
        {
            // release the touch object.
            touch->release();
        }
    }

    void GLView::handleTouchesEnd(int num, intptr_t ids[], float xs[], float ys[])
    {
        handleTouchesOfEndOrCancel(EventTouch::EventCode::ENDED, num, ids, xs, ys);
    }

    void GLView::handleTouchesCancel(int num, intptr_t ids[], float xs[], float ys[])
    {
        handleTouchesOfEndOrCancel(EventTouch::EventCode::CANCELLED, num, ids, xs, ys);
    }

    const MATH::Rectf& GLView::getViewPortRect() const
    {
        return _viewPortRect;
    }

    std::vector<Touch*> GLView::getAllTouches() const
    {
        return getAllTouchesVector();
    }

    float GLView::getScaleX() const
    {
        return _scaleX;
    }

    float GLView::getScaleY() const
    {
        return _scaleY;
    }

    class GLFWEventHandler
    {
    public:
        static void onGLFWError(int errorID, const char* errorDesc)
        {
            if (_view)
                _view->onGLFWError(errorID, errorDesc);
        }

        static void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify)
        {
            if (_view)
                _view->onGLFWMouseCallBack(window, button, action, modify);
        }

        static void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
        {
            if (_view)
                _view->onGLFWMouseMoveCallBack(window, x, y);
        }

        static void onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y)
        {
            if (_view)
                _view->onGLFWMouseScrollCallback(window, x, y);
        }

        static void onGLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (_view)
                _view->onGLFWKeyCallback(window, key, scancode, action, mods);
        }

        static void onGLFWCharCallback(GLFWwindow* window, unsigned int character)
        {
            if (_view)
                _view->onGLFWCharCallback(window, character);
        }

        static void onGLFWWindowPosCallback(GLFWwindow* windows, int x, int y)
        {
            if (_view)
                _view->onGLFWWindowPosCallback(windows, x, y);
        }

        static void onGLFWframebuffersize(GLFWwindow* window, int w, int h)
        {
            if (_view)
                _view->onGLFWframebuffersize(window, w, h);
        }

        static void onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height)
        {
            if (_view)
                _view->onGLFWWindowSizeFunCallback(window, width, height);
        }

        static void setGLViewImpl(GLViewImpl* view)
        {
            _view = view;
        }

    private:
        static GLViewImpl* _view;
    };

    GLViewImpl* GLFWEventHandler::_view = nullptr;

    ////////////////////////////////////////////////////

    struct keyCodeItem
    {
        int glfwKeyCode;
        EventKeyboard::KeyCode keyCode;
    };

    static std::unordered_map<int, EventKeyboard::KeyCode> g_keyCodeMap;

    static keyCodeItem g_keyCodeStructArray[] = {
        /* The unknown key */
        { GLFW_KEY_UNKNOWN         , EventKeyboard::KeyCode::KEY_NONE          },

        /* Printable keys */
        { GLFW_KEY_SPACE           , EventKeyboard::KeyCode::KEY_SPACE         },
        { GLFW_KEY_APOSTROPHE      , EventKeyboard::KeyCode::KEY_APOSTROPHE    },
        { GLFW_KEY_COMMA           , EventKeyboard::KeyCode::KEY_COMMA         },
        { GLFW_KEY_MINUS           , EventKeyboard::KeyCode::KEY_MINUS         },
        { GLFW_KEY_PERIOD          , EventKeyboard::KeyCode::KEY_PERIOD        },
        { GLFW_KEY_SLASH           , EventKeyboard::KeyCode::KEY_SLASH         },
        { GLFW_KEY_0               , EventKeyboard::KeyCode::KEY_0             },
        { GLFW_KEY_1               , EventKeyboard::KeyCode::KEY_1             },
        { GLFW_KEY_2               , EventKeyboard::KeyCode::KEY_2             },
        { GLFW_KEY_3               , EventKeyboard::KeyCode::KEY_3             },
        { GLFW_KEY_4               , EventKeyboard::KeyCode::KEY_4             },
        { GLFW_KEY_5               , EventKeyboard::KeyCode::KEY_5             },
        { GLFW_KEY_6               , EventKeyboard::KeyCode::KEY_6             },
        { GLFW_KEY_7               , EventKeyboard::KeyCode::KEY_7             },
        { GLFW_KEY_8               , EventKeyboard::KeyCode::KEY_8             },
        { GLFW_KEY_9               , EventKeyboard::KeyCode::KEY_9             },
        { GLFW_KEY_SEMICOLON       , EventKeyboard::KeyCode::KEY_SEMICOLON     },
        { GLFW_KEY_EQUAL           , EventKeyboard::KeyCode::KEY_EQUAL         },
        { GLFW_KEY_A               , EventKeyboard::KeyCode::KEY_A             },
        { GLFW_KEY_B               , EventKeyboard::KeyCode::KEY_B             },
        { GLFW_KEY_C               , EventKeyboard::KeyCode::KEY_C             },
        { GLFW_KEY_D               , EventKeyboard::KeyCode::KEY_D             },
        { GLFW_KEY_E               , EventKeyboard::KeyCode::KEY_E             },
        { GLFW_KEY_F               , EventKeyboard::KeyCode::KEY_F             },
        { GLFW_KEY_G               , EventKeyboard::KeyCode::KEY_G             },
        { GLFW_KEY_H               , EventKeyboard::KeyCode::KEY_H             },
        { GLFW_KEY_I               , EventKeyboard::KeyCode::KEY_I             },
        { GLFW_KEY_J               , EventKeyboard::KeyCode::KEY_J             },
        { GLFW_KEY_K               , EventKeyboard::KeyCode::KEY_K             },
        { GLFW_KEY_L               , EventKeyboard::KeyCode::KEY_L             },
        { GLFW_KEY_M               , EventKeyboard::KeyCode::KEY_M             },
        { GLFW_KEY_N               , EventKeyboard::KeyCode::KEY_N             },
        { GLFW_KEY_O               , EventKeyboard::KeyCode::KEY_O             },
        { GLFW_KEY_P               , EventKeyboard::KeyCode::KEY_P             },
        { GLFW_KEY_Q               , EventKeyboard::KeyCode::KEY_Q             },
        { GLFW_KEY_R               , EventKeyboard::KeyCode::KEY_R             },
        { GLFW_KEY_S               , EventKeyboard::KeyCode::KEY_S             },
        { GLFW_KEY_T               , EventKeyboard::KeyCode::KEY_T             },
        { GLFW_KEY_U               , EventKeyboard::KeyCode::KEY_U             },
        { GLFW_KEY_V               , EventKeyboard::KeyCode::KEY_V             },
        { GLFW_KEY_W               , EventKeyboard::KeyCode::KEY_W             },
        { GLFW_KEY_X               , EventKeyboard::KeyCode::KEY_X             },
        { GLFW_KEY_Y               , EventKeyboard::KeyCode::KEY_Y             },
        { GLFW_KEY_Z               , EventKeyboard::KeyCode::KEY_Z             },
        { GLFW_KEY_LEFT_BRACKET    , EventKeyboard::KeyCode::KEY_LEFT_BRACKET  },
        { GLFW_KEY_BACKSLASH       , EventKeyboard::KeyCode::KEY_BACK_SLASH    },
        { GLFW_KEY_RIGHT_BRACKET   , EventKeyboard::KeyCode::KEY_RIGHT_BRACKET },
        { GLFW_KEY_GRAVE_ACCENT    , EventKeyboard::KeyCode::KEY_GRAVE         },
        { GLFW_KEY_WORLD_1         , EventKeyboard::KeyCode::KEY_GRAVE         },
        { GLFW_KEY_WORLD_2         , EventKeyboard::KeyCode::KEY_NONE          },

        /* Function keys */
        { GLFW_KEY_ESCAPE          , EventKeyboard::KeyCode::KEY_ESCAPE        },
        { GLFW_KEY_ENTER           , EventKeyboard::KeyCode::KEY_KP_ENTER      },
        { GLFW_KEY_TAB             , EventKeyboard::KeyCode::KEY_TAB           },
        { GLFW_KEY_BACKSPACE       , EventKeyboard::KeyCode::KEY_BACKSPACE     },
        { GLFW_KEY_INSERT          , EventKeyboard::KeyCode::KEY_INSERT        },
        { GLFW_KEY_DELETE          , EventKeyboard::KeyCode::KEY_DELETE        },
        { GLFW_KEY_RIGHT           , EventKeyboard::KeyCode::KEY_RIGHT_ARROW   },
        { GLFW_KEY_LEFT            , EventKeyboard::KeyCode::KEY_LEFT_ARROW    },
        { GLFW_KEY_DOWN            , EventKeyboard::KeyCode::KEY_DOWN_ARROW    },
        { GLFW_KEY_UP              , EventKeyboard::KeyCode::KEY_UP_ARROW      },
        { GLFW_KEY_PAGE_UP         , EventKeyboard::KeyCode::KEY_KP_PG_UP      },
        { GLFW_KEY_PAGE_DOWN       , EventKeyboard::KeyCode::KEY_KP_PG_DOWN    },
        { GLFW_KEY_HOME            , EventKeyboard::KeyCode::KEY_KP_HOME       },
        { GLFW_KEY_END             , EventKeyboard::KeyCode::KEY_END           },
        { GLFW_KEY_CAPS_LOCK       , EventKeyboard::KeyCode::KEY_CAPS_LOCK     },
        { GLFW_KEY_SCROLL_LOCK     , EventKeyboard::KeyCode::KEY_SCROLL_LOCK   },
        { GLFW_KEY_NUM_LOCK        , EventKeyboard::KeyCode::KEY_NUM_LOCK      },
        { GLFW_KEY_PRINT_SCREEN    , EventKeyboard::KeyCode::KEY_PRINT         },
        { GLFW_KEY_PAUSE           , EventKeyboard::KeyCode::KEY_PAUSE         },
        { GLFW_KEY_F1              , EventKeyboard::KeyCode::KEY_F1            },
        { GLFW_KEY_F2              , EventKeyboard::KeyCode::KEY_F2            },
        { GLFW_KEY_F3              , EventKeyboard::KeyCode::KEY_F3            },
        { GLFW_KEY_F4              , EventKeyboard::KeyCode::KEY_F4            },
        { GLFW_KEY_F5              , EventKeyboard::KeyCode::KEY_F5            },
        { GLFW_KEY_F6              , EventKeyboard::KeyCode::KEY_F6            },
        { GLFW_KEY_F7              , EventKeyboard::KeyCode::KEY_F7            },
        { GLFW_KEY_F8              , EventKeyboard::KeyCode::KEY_F8            },
        { GLFW_KEY_F9              , EventKeyboard::KeyCode::KEY_F9            },
        { GLFW_KEY_F10             , EventKeyboard::KeyCode::KEY_F10           },
        { GLFW_KEY_F11             , EventKeyboard::KeyCode::KEY_F11           },
        { GLFW_KEY_F12             , EventKeyboard::KeyCode::KEY_F12           },
        { GLFW_KEY_F13             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F14             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F15             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F16             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F17             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F18             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F19             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F20             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F21             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F22             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F23             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F24             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_F25             , EventKeyboard::KeyCode::KEY_NONE          },
        { GLFW_KEY_KP_0            , EventKeyboard::KeyCode::KEY_0             },
        { GLFW_KEY_KP_1            , EventKeyboard::KeyCode::KEY_1             },
        { GLFW_KEY_KP_2            , EventKeyboard::KeyCode::KEY_2             },
        { GLFW_KEY_KP_3            , EventKeyboard::KeyCode::KEY_3             },
        { GLFW_KEY_KP_4            , EventKeyboard::KeyCode::KEY_4             },
        { GLFW_KEY_KP_5            , EventKeyboard::KeyCode::KEY_5             },
        { GLFW_KEY_KP_6            , EventKeyboard::KeyCode::KEY_6             },
        { GLFW_KEY_KP_7            , EventKeyboard::KeyCode::KEY_7             },
        { GLFW_KEY_KP_8            , EventKeyboard::KeyCode::KEY_8             },
        { GLFW_KEY_KP_9            , EventKeyboard::KeyCode::KEY_9             },
        { GLFW_KEY_KP_DECIMAL      , EventKeyboard::KeyCode::KEY_PERIOD        },
        { GLFW_KEY_KP_DIVIDE       , EventKeyboard::KeyCode::KEY_KP_DIVIDE     },
        { GLFW_KEY_KP_MULTIPLY     , EventKeyboard::KeyCode::KEY_KP_MULTIPLY   },
        { GLFW_KEY_KP_SUBTRACT     , EventKeyboard::KeyCode::KEY_KP_MINUS      },
        { GLFW_KEY_KP_ADD          , EventKeyboard::KeyCode::KEY_KP_PLUS       },
        { GLFW_KEY_KP_ENTER        , EventKeyboard::KeyCode::KEY_KP_ENTER      },
        { GLFW_KEY_KP_EQUAL        , EventKeyboard::KeyCode::KEY_EQUAL         },
        { GLFW_KEY_LEFT_SHIFT      , EventKeyboard::KeyCode::KEY_LEFT_SHIFT         },
        { GLFW_KEY_LEFT_CONTROL    , EventKeyboard::KeyCode::KEY_LEFT_CTRL          },
        { GLFW_KEY_LEFT_ALT        , EventKeyboard::KeyCode::KEY_LEFT_ALT           },
        { GLFW_KEY_LEFT_SUPER      , EventKeyboard::KeyCode::KEY_HYPER         },
        { GLFW_KEY_RIGHT_SHIFT     , EventKeyboard::KeyCode::KEY_RIGHT_SHIFT         },
        { GLFW_KEY_RIGHT_CONTROL   , EventKeyboard::KeyCode::KEY_RIGHT_CTRL          },
        { GLFW_KEY_RIGHT_ALT       , EventKeyboard::KeyCode::KEY_RIGHT_ALT           },
        { GLFW_KEY_RIGHT_SUPER     , EventKeyboard::KeyCode::KEY_HYPER         },
        { GLFW_KEY_MENU            , EventKeyboard::KeyCode::KEY_MENU          },
        { GLFW_KEY_LAST            , EventKeyboard::KeyCode::KEY_NONE          }
    };

    //////////////////////////////////////////////////////////////////////////
    // implement GLViewImpl
    //////////////////////////////////////////////////////////////////////////


    GLViewImpl::GLViewImpl()
    : _captured(false)
    , _supportTouch(false)
    , _isInRetinaMonitor(false)
    , _isRetinaEnabled(false)
    , _retinaFactor(1)
    , _frameZoomFactor(1.0f)
    , _mainWindow(nullptr)
    , _monitor(nullptr)
    , _mouseX(0.0f)
    , _mouseY(0.0f)
    {
        _viewName = "cocos2dx";
        g_keyCodeMap.clear();
        for (auto& item : g_keyCodeStructArray)
        {
            g_keyCodeMap[item.glfwKeyCode] = item.keyCode;
        }

        GLFWEventHandler::setGLViewImpl(this);

        glfwSetErrorCallback(GLFWEventHandler::onGLFWError);
        glfwInit();
    }

    GLViewImpl::~GLViewImpl()
    {
        GLFWEventHandler::setGLViewImpl(nullptr);
        glfwTerminate();
    }

    GLViewImpl* GLViewImpl::create(const std::string& viewName)
    {
        auto ret = new (std::nothrow) GLViewImpl;
        if(ret && ret->initWithRect(viewName, MATH::Rectf(0, 0, 960, 640), 1)) {
            ret->autorelease();
            return ret;
        }

        return nullptr;
    }

    GLViewImpl* GLViewImpl::createWithRect(const std::string& viewName, MATH::Rectf rect, float frameZoomFactor)
    {
        auto ret = new (std::nothrow) GLViewImpl;
        if(ret && ret->initWithRect(viewName, rect, frameZoomFactor)) {
            ret->autorelease();
            return ret;
        }

        return nullptr;
    }

    GLViewImpl* GLViewImpl::createWithFullScreen(const std::string& viewName)
    {
        auto ret = new (std::nothrow) GLViewImpl();
        if(ret && ret->initWithFullScreen(viewName)) {
            ret->autorelease();
            return ret;
        }

        return nullptr;
    }

    GLViewImpl* GLViewImpl::createWithFullScreen(const std::string& viewName, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
    {
        auto ret = new (std::nothrow) GLViewImpl();
        if(ret && ret->initWithFullscreen(viewName, videoMode, monitor)) {
            ret->autorelease();
            return ret;
        }

        return nullptr;
    }

    bool GLViewImpl::initWithRect(const std::string& viewName, MATH::Rectf rect, float frameZoomFactor)
    {
        setViewName(viewName);

        _frameZoomFactor = frameZoomFactor;

        glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
        glfwWindowHint(GLFW_RED_BITS,_glContextAttrs.redBits);
        glfwWindowHint(GLFW_GREEN_BITS,_glContextAttrs.greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,_glContextAttrs.blueBits);
        glfwWindowHint(GLFW_ALPHA_BITS,_glContextAttrs.alphaBits);
        glfwWindowHint(GLFW_DEPTH_BITS,_glContextAttrs.depthBits);
        glfwWindowHint(GLFW_STENCIL_BITS,_glContextAttrs.stencilBits);

        _mainWindow = glfwCreateWindow(rect.size.width * _frameZoomFactor,
                                       rect.size.height * _frameZoomFactor,
                                       _viewName.c_str(),
                                       _monitor,
                                       nullptr);
        glfwMakeContextCurrent(_mainWindow);

        glfwSetMouseButtonCallback(_mainWindow, GLFWEventHandler::onGLFWMouseCallBack);
        glfwSetCursorPosCallback(_mainWindow, GLFWEventHandler::onGLFWMouseMoveCallBack);
        glfwSetScrollCallback(_mainWindow, GLFWEventHandler::onGLFWMouseScrollCallback);
        glfwSetCharCallback(_mainWindow, GLFWEventHandler::onGLFWCharCallback);
        glfwSetKeyCallback(_mainWindow, GLFWEventHandler::onGLFWKeyCallback);
        glfwSetWindowPosCallback(_mainWindow, GLFWEventHandler::onGLFWWindowPosCallback);
        glfwSetFramebufferSizeCallback(_mainWindow, GLFWEventHandler::onGLFWframebuffersize);
        glfwSetWindowSizeCallback(_mainWindow, GLFWEventHandler::onGLFWWindowSizeFunCallback);

        setFrameSize(rect.size.width, rect.size.height);

        // check OpenGL version at first
        const GLubyte* glVersion = glGetString(GL_VERSION);

        if (atof((const char*)glVersion) < 1.5 )
        {
            char strComplain[256] = {0};
            sprintf(strComplain,
                    "OpenGL 1.5 or higher is required (your version is %s). Please upgrade the driver of your video card.",
                    glVersion);
            return false;
        }

        initGlew();

        // Enable point size by default.
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        return true;
    }

    bool GLViewImpl::initWithFullScreen(const std::string& viewName)
    {
        //Create fullscreen window on primary monitor at its current video mode.
        _monitor = glfwGetPrimaryMonitor();
        if (nullptr == _monitor)
            return false;

        const GLFWvidmode* videoMode = glfwGetVideoMode(_monitor);
        return initWithRect(viewName, MATH::Rectf(0, 0, videoMode->width, videoMode->height), 1.0f);
    }

    bool GLViewImpl::initWithFullscreen(const std::string &viewname, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
    {
        //Create fullscreen on specified monitor at the specified video mode.
        _monitor = monitor;
        if (nullptr == _monitor)
            return false;

        //These are soft contraints. If the video mode is retrieved at runtime, the resulting window and context should match these exactly. If invalid attribs are passed (eg. from an outdated cache), window creation will NOT fail but the actual window/context may differ.
        glfwWindowHint(GLFW_REFRESH_RATE, videoMode.refreshRate);
        glfwWindowHint(GLFW_RED_BITS, videoMode.redBits);
        glfwWindowHint(GLFW_BLUE_BITS, videoMode.blueBits);
        glfwWindowHint(GLFW_GREEN_BITS, videoMode.greenBits);

        return initWithRect(viewname, MATH::Rectf(0, 0, videoMode.width, videoMode.height), 1.0f);
    }

    bool GLViewImpl::isOpenGLReady()
    {
        return nullptr != _mainWindow;
    }

    void GLViewImpl::end()
    {
        if(_mainWindow)
        {
            glfwSetWindowShouldClose(_mainWindow,1);
            _mainWindow = nullptr;
        }
        // Release self. Otherwise, GLViewImpl could not be freed.
        release();
    }

    void GLViewImpl::swapBuffers()
    {
        if(_mainWindow)
            glfwSwapBuffers(_mainWindow);
    }

    bool GLViewImpl::windowShouldClose()
    {
        if(_mainWindow)
            return glfwWindowShouldClose(_mainWindow) ? true : false;
        else
            return true;
    }

    void GLViewImpl::pollEvents()
    {
        glfwPollEvents();
    }

    void GLViewImpl::setCursorVisible( bool isVisible )
    {
        if( _mainWindow == NULL )
            return;

        if( isVisible )
            glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void GLViewImpl::setFrameZoomFactor(float zoomFactor)
    {
        if (fabs(_frameZoomFactor - zoomFactor) < FLT_EPSILON)
        {
            return;
        }

        _frameZoomFactor = zoomFactor;
        updateFrameSize();
    }

    float GLViewImpl::getFrameZoomFactor() const
    {
        return _frameZoomFactor;
    }

    void GLViewImpl::updateFrameSize()
    {
        if (_screenSize.width > 0 && _screenSize.height > 0)
        {
            int w = 0, h = 0;
            glfwGetWindowSize(_mainWindow, &w, &h);

            int frameBufferW = 0, frameBufferH = 0;
            glfwGetFramebufferSize(_mainWindow, &frameBufferW, &frameBufferH);

            if (frameBufferW == 2 * w && frameBufferH == 2 * h)
            {
                if (_isRetinaEnabled)
                {
                    _retinaFactor = 1;
                }
                else
                {
                    _retinaFactor = 2;
                }
                glfwSetWindowSize(_mainWindow, _screenSize.width/2 * _retinaFactor * _frameZoomFactor, _screenSize.height/2 * _retinaFactor * _frameZoomFactor);

                _isInRetinaMonitor = true;
            }
            else
            {
                if (_isInRetinaMonitor)
                {
                    _retinaFactor = 1;
                }
                glfwSetWindowSize(_mainWindow, _screenSize.width * _retinaFactor * _frameZoomFactor, _screenSize.height *_retinaFactor * _frameZoomFactor);

                _isInRetinaMonitor = false;
            }
        }
    }

    void GLViewImpl::setFrameSize(float width, float height)
    {
        GLView::setFrameSize(width, height);
        updateFrameSize();
    }

    void GLViewImpl::setViewPortInPoints(float x , float y , float w , float h)
    {
        Viewport vp = {(float)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
            (float)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
            (float)(w * _scaleX * _retinaFactor * _frameZoomFactor),
            (float)(h * _scaleY * _retinaFactor * _frameZoomFactor)};
        Camera::setDefaultViewport(vp);
    }

    void GLViewImpl::setScissorInPoints(float x , float y , float w , float h)
    {
        glScissor((GLint)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor),
                   (GLint)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor),
                   (GLsizei)(w * _scaleX * _retinaFactor * _frameZoomFactor),
                   (GLsizei)(h * _scaleY * _retinaFactor * _frameZoomFactor));
    }

    void GLViewImpl::onGLFWError(int errorID, const char* errorDesc)
    {
        throw _HException_Normal(UTILS::STRING::StringFromFormat("GLFWError #%d Happen, %s\n", errorID, errorDesc));
    }

    void GLViewImpl::onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int modify)
    {
        if(GLFW_MOUSE_BUTTON_LEFT == button)
        {
            if(GLFW_PRESS == action)
            {
                _captured = true;
                if (this->getViewPortRect().equals(MATH::RectfZERO) || this->getViewPortRect().contains(MATH::Vector2f(_mouseX,_mouseY)))
                {
                    intptr_t id = 0;
                    this->handleTouchesBegin(1, &id, &_mouseX, &_mouseY);
                }
            }
            else if(GLFW_RELEASE == action)
            {
                if (_captured)
                {
                    _captured = false;
                    intptr_t id = 0;
                    this->handleTouchesEnd(1, &id, &_mouseX, &_mouseY);
                }
            }
        }

        //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
        float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
        float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;

        if(GLFW_PRESS == action)
        {
            EventMouse event(EventMouse::MouseEventType::MOUSE_DOWN);
            event.setCursorPosition(cursorX, cursorY);
            event.setMouseButton(button);
            Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        }
        else if(GLFW_RELEASE == action)
        {
            EventMouse event(EventMouse::MouseEventType::MOUSE_UP);
            event.setCursorPosition(cursorX, cursorY);
            event.setMouseButton(button);
            Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        }
    }

    void GLViewImpl::onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
    {
        _mouseX = (float)x;
        _mouseY = (float)y;

        _mouseX /= this->getFrameZoomFactor();
        _mouseY /= this->getFrameZoomFactor();

        if (_isInRetinaMonitor)
        {
            if (_retinaFactor == 1)
            {
                _mouseX *= 2;
                _mouseY *= 2;
            }
        }

        if (_captured)
        {
            intptr_t id = 0;
            this->handleTouchesMove(1, &id, &_mouseX, &_mouseY);
        }

        //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
        float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
        float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;

        EventMouse event(EventMouse::MouseEventType::MOUSE_MOVE);
        // Set current button
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            event.setMouseButton(GLFW_MOUSE_BUTTON_LEFT);
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            event.setMouseButton(GLFW_MOUSE_BUTTON_RIGHT);
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
        {
            event.setMouseButton(GLFW_MOUSE_BUTTON_MIDDLE);
        }
        event.setCursorPosition(cursorX, cursorY);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }

    void GLViewImpl::onGLFWMouseScrollCallback(GLFWwindow* window, double x, double y)
    {
        EventMouse event(EventMouse::MouseEventType::MOUSE_SCROLL);
        //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
        float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
        float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
        event.setScrollData((float)x, -(float)y);
        event.setCursorPosition(cursorX, cursorY);
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }

    void GLViewImpl::onGLFWKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (GLFW_REPEAT != action)
        {
            EventKeyboard event(g_keyCodeMap[key], GLFW_PRESS == action);
            auto dispatcher = Director::getInstance()->getEventDispatcher();
            dispatcher->dispatchEvent(&event);
        }

        if (GLFW_RELEASE != action && g_keyCodeMap[key] == EventKeyboard::KeyCode::KEY_BACKSPACE)
        {
            IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
        }
    }

    void GLViewImpl::onGLFWCharCallback(GLFWwindow *window, unsigned int character)
    {
        char16_t wcharString[2] = { (char16_t) character, 0 };
        std::string utf8String;

        UTILS::STRING::UTF16ToUTF8(wcharString, utf8String);
        IMEDispatcher::sharedDispatcher()->dispatchInsertText( utf8String.c_str(), utf8String.size() );
    }

    void GLViewImpl::onGLFWWindowPosCallback(GLFWwindow *windows, int x, int y)
    {
        Director::getInstance()->setViewport();
    }

    void GLViewImpl::onGLFWframebuffersize(GLFWwindow* window, int w, int h)
    {
        float frameSizeW = _screenSize.width;
        float frameSizeH = _screenSize.height;
        float factorX = frameSizeW / w * _retinaFactor * _frameZoomFactor;
        float factorY = frameSizeH / h * _retinaFactor * _frameZoomFactor;

        if (fabs(factorX - 0.5f) < FLT_EPSILON && fabs(factorY - 0.5f) < FLT_EPSILON )
        {
            _isInRetinaMonitor = true;
            if (_isRetinaEnabled)
            {
                _retinaFactor = 1;
            }
            else
            {
                _retinaFactor = 2;
            }

            glfwSetWindowSize(window, static_cast<int>(frameSizeW * 0.5f * _retinaFactor * _frameZoomFactor) , static_cast<int>(frameSizeH * 0.5f * _retinaFactor * _frameZoomFactor));
        }
        else if(fabs(factorX - 2.0f) < FLT_EPSILON && fabs(factorY - 2.0f) < FLT_EPSILON)
        {
            _isInRetinaMonitor = false;
            _retinaFactor = 1;
            glfwSetWindowSize(window, static_cast<int>(frameSizeW * _retinaFactor * _frameZoomFactor), static_cast<int>(frameSizeH * _retinaFactor * _frameZoomFactor));
        }
    }

    void GLViewImpl::onGLFWWindowSizeFunCallback(GLFWwindow *window, int width, int height)
    {
        if (_resolutionPolicy != ResolutionPolicy::UNKNOWN)
        {
            updateDesignResolutionSize();
            Director::getInstance()->setViewport();
        }
    }

    #ifdef _WIN32
    static bool glew_dynamic_binding()
    {
        const char *gl_extensions = (const char*)glGetString(GL_EXTENSIONS);

        // If the current opengl driver doesn't have framebuffers methods, check if an extension exists
        if (glGenFramebuffers == nullptr)
        {
            if (strstr(gl_extensions, "ARB_framebuffer_object"))
            {
                glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) wglGetProcAddress("glIsRenderbuffer");
                glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
                glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffers");
                glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
                glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
                glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) wglGetProcAddress("glGetRenderbufferParameteriv");
                glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) wglGetProcAddress("glIsFramebuffer");
                glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
                glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
                glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
                glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
                glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) wglGetProcAddress("glFramebufferTexture1D");
                glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
                glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) wglGetProcAddress("glFramebufferTexture3D");
                glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
                glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
                glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
            }
            else
            if (strstr(gl_extensions, "EXT_framebuffer_object"))
            {
                glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) wglGetProcAddress("glIsRenderbufferEXT");
                glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbufferEXT");
                glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffersEXT");
                glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffersEXT");
                glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorageEXT");
                glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) wglGetProcAddress("glGetRenderbufferParameterivEXT");
                glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) wglGetProcAddress("glIsFramebufferEXT");
                glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebufferEXT");
                glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffersEXT");
                glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffersEXT");
                glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
                glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) wglGetProcAddress("glFramebufferTexture1DEXT");
                glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
                glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) wglGetProcAddress("glFramebufferTexture3DEXT");
                glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
                glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
                glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmapEXT");
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    #endif

    // helper
    void GLViewImpl::initGlew()
    {
        GLenum GlewInitResult = glewInit();
        if (GLEW_OK != GlewInitResult)
        {
            throw _HException_Normal((char *)glewGetErrorString(GlewInitResult));
        }

        if (!(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader))
        {
            throw _HException_Normal("Not totally ready :(");
        }

        if (!glewIsSupported("GL_VERSION_2_0"))
        {
            throw _HException_Normal("OpenGL 2.0 not supported");
        }

    #ifdef _WIN32
        if(glew_dynamic_binding() == false)
        {
            throw _HException_Normal("No OpenGL framebuffer support. Please upgrade the driver of your video card.");
        }
    #endif
    }
}
