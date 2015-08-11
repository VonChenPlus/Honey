#include "GRAPH/BASE/Director.h"
#include <string>
#include "MATH/Matrix.h"
#include "UTILS/TIME/TimeUtils.h"
#include "GRAPH/BASE/Scheduler.h"
#include "GRAPH/BASE/ActionManager.h"
#include "GRAPH/BASE/EventDispatcher.h"
#include "GRAPH/BASE/Event.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "GRAPH/RENDERER/RenderState.h"
#include "GRAPH/BASE/Configuration.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/RENDERER/FrameBuffer.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "GRAPH/RENDERER/GLProgramCache.h"
#include "GRAPH/RENDERER/TextureCache.h"
#include "GRAPH/RENDERER/GLProgramState.h"
#include "BASE/AutoreleasePool.h"
#include "IO/FileUtils.h"
#undef max

namespace GRAPH
{
    /**
     Position of the FPS

     Default: 0,0 (bottom-left corner)
     */
    #ifndef CC_DIRECTOR_STATS_POSITION
    #define CC_DIRECTOR_STATS_POSITION Director::getInstance()->getVisibleOrigin()
    #endif // CC_DIRECTOR_STATS_POSITION

    using namespace std;

    // FIXME: it should be a Director ivar. Move it there once support for multiple directors is added

    // singleton stuff
    static DisplayLinkDirector *s_SharedDirector = nullptr;

    #define kDefaultFPS        60  // 60 frames per second
    extern const char* cocos2dVersion(void);

    const char *Director::EVENT_PROJECTION_CHANGED = "director_projection_changed";
    const char *Director::EVENT_AFTER_DRAW = "director_after_draw";
    const char *Director::EVENT_AFTER_VISIT = "director_after_visit";
    const char *Director::EVENT_AFTER_UPDATE = "director_after_update";

    Director* Director::getInstance()
    {
        if (!s_SharedDirector)
        {
            s_SharedDirector = new (std::nothrow) DisplayLinkDirector();
            s_SharedDirector->init();
        }

        return s_SharedDirector;
    }

    Director::Director()
    : _isStatusLabelUpdated(true)
    {
    }

    bool Director::init(void)
    {
        setDefaultValues();

        // scenes
        _runningScene = nullptr;
        _nextScene = nullptr;

        _notificationNode = nullptr;

        _scenesStack.reserve(15);

        // FPS
        _accumDt = 0.0f;
        _frameRate = 0.0f;
        _totalFrames = 0;
        _lastUpdate = new ::timeval();
        _secondsPerFrame = 1.0f;

        // paused ?
        _paused = false;

        // purge ?
        _purgeDirectorInNextLoop = false;

        // restart ?
        _restartDirectorInNextLoop = false;

        _winSizeInPoints = MATH::SizefZERO;

        _openGLView = nullptr;
        _defaultFBO = nullptr;

        _contentScaleFactor = 1.0f;

        // scheduler
        _scheduler = new (std::nothrow) Scheduler();
        // action manager
        _actionManager = new (std::nothrow) ActionManager();
        _scheduler->scheduleUpdate(_actionManager, Scheduler::PRIORITY_SYSTEM, false);

        _eventDispatcher = new (std::nothrow) EventDispatcher();
        _eventAfterDraw = new (std::nothrow) EventCustom(EVENT_AFTER_DRAW);
        _eventAfterDraw->setUserData(this);
        _eventAfterVisit = new (std::nothrow) EventCustom(EVENT_AFTER_VISIT);
        _eventAfterVisit->setUserData(this);
        _eventAfterUpdate = new (std::nothrow) EventCustom(EVENT_AFTER_UPDATE);
        _eventAfterUpdate->setUserData(this);
        _eventProjectionChanged = new (std::nothrow) EventCustom(EVENT_PROJECTION_CHANGED);
        _eventProjectionChanged->setUserData(this);
        //init TextureCache
        initTextureCache();
        initMatrixStack();

        _renderer = new (std::nothrow) Renderer;
        RenderState::initialize();

        return true;
    }

    Director::~Director(void)
    {
        SAFE_RELEASE(_runningScene);
        SAFE_RELEASE(_notificationNode);
        SAFE_RELEASE(_scheduler);
        SAFE_RELEASE(_actionManager);
        SAFE_DELETE(_defaultFBO);

        delete _eventAfterUpdate;
        delete _eventAfterDraw;
        delete _eventAfterVisit;
        delete _eventProjectionChanged;

        delete _renderer;

        SAFE_RELEASE(_eventDispatcher);

        // delete _lastUpdate
        SAFE_DELETE(_lastUpdate);

        Configuration::destroyInstance();

        s_SharedDirector = nullptr;
    }

    void Director::setDefaultValues(void)
    {
        Configuration *conf = Configuration::getInstance();

        // default FPS
        double fps = conf->getValue("cocos2d.x.fps", HValue(kDefaultFPS)).asDouble();
        _oldAnimationInterval = _animationInterval = 1.0 / fps;

        // GL projection
        std::string projection = conf->getValue("cocos2d.x.gl.projection", HValue("3d")).asString();
        if (projection == "3d")
            _projection = Projection::_3D;
        else if (projection == "2d")
            _projection = Projection::_2D;
        else if (projection == "custom")
            _projection = Projection::CUSTOM;

        // Default pixel format for PNG images with alpha
        std::string pixel_format = conf->getValue("cocos2d.x.texture.pixel_format_for_png", HValue("rgba8888")).asString();
        if (pixel_format == "rgba8888")
            Texture2D::setDefaultAlphaPixelFormat(IMAGE::PixelFormat::RGBA8888);
        else if(pixel_format == "rgba4444")
            Texture2D::setDefaultAlphaPixelFormat(IMAGE::PixelFormat::RGBA4444);
        else if(pixel_format == "rgba5551")
            Texture2D::setDefaultAlphaPixelFormat(IMAGE::PixelFormat::RGB5A1);
    }

    void Director::setGLDefaultValues()
    {
        // This method SHOULD be called only after openGLView_ was initialized
        setAlphaBlending(true);
        setDepthTest(false);
        setProjection(_projection);
    }

    // Draw the Scene
    void Director::drawScene()
    {
        // calculate "global" dt
        calculateDeltaTime();

        if (_openGLView)
        {
            _openGLView->pollEvents();
        }

        //tick before glClear: issue #533
        if (! _paused)
        {
            _scheduler->update(_deltaTime);
            _eventDispatcher->dispatchEvent(_eventAfterUpdate);
        }

        _renderer->clear();
        FrameBuffer::clearAllFBOs();
        /* to avoid flickr, nextScene MUST be here: after tick and before draw.
         * FIXME: Which bug is this one. It seems that it can't be reproduced with v0.9
         */
        if (_nextScene)
        {
            setNextScene();
        }

        pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

        if (_runningScene)
        {
            //clear draw stats
            _renderer->clearDrawStats();

            //render the scene
            _runningScene->render(_renderer);

            _eventDispatcher->dispatchEvent(_eventAfterVisit);
        }

        // draw the notifications node
        if (_notificationNode)
        {
            _notificationNode->visit(_renderer, MATH::Matrix4::IDENTITY, 0);
        }

        _renderer->render();

        _eventDispatcher->dispatchEvent(_eventAfterDraw);

        popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

        _totalFrames++;

        // swap buffers
        if (_openGLView)
        {
            _openGLView->swapBuffers();
        }
    }

    void Director::calculateDeltaTime()
    {
        ::timeval now;

        if (UTILS::TIME::GetTimeOfDay(&now, nullptr) != 0)
        {
            _deltaTime = 0;
            return;
        }

        // new delta time. Re-fixed issue #1277
        if (_nextDeltaTimeZero)
        {
            _deltaTime = 0;
            _nextDeltaTimeZero = false;
        }
        else
        {
            _deltaTime = (now.tv_sec - _lastUpdate->tv_sec) + (now.tv_usec - _lastUpdate->tv_usec) / 1000000.0f;
            _deltaTime = std::max(0.0f, _deltaTime);
        }

        *_lastUpdate = now;
    }

    float Director::getDeltaTime() const
    {
        return _deltaTime;
    }

    void Director::setOpenGLView(GLView *openGLView)
    {
        if (_openGLView != openGLView)
        {
            // Configuration. Gather GPU info
            Configuration *conf = Configuration::getInstance();
            conf->gatherGPUInfo();

            if(_openGLView)
                _openGLView->release();
            _openGLView = openGLView;
            _openGLView->retain();

            // set size
            _winSizeInPoints = _openGLView->getDesignResolutionSize();

            _isStatusLabelUpdated = true;

            if (_openGLView)
            {
                setGLDefaultValues();
            }

            _renderer->initGLView();

            if (_eventDispatcher)
            {
                _eventDispatcher->setEnabled(true);
            }

            _defaultFBO = FrameBuffer::getOrCreateDefaultFBO(_openGLView);
            _defaultFBO->retain();
        }
    }

    TextureCache* Director::getTextureCache() const
    {
        return _textureCache;
    }

    void Director::initTextureCache()
    {
        _textureCache = new (std::nothrow) TextureCache();
    }

    void Director::destroyTextureCache()
    {
        if (_textureCache)
        {
            _textureCache->waitForQuit();
            SAFE_RELEASE_NULL(_textureCache);
        }
    }

    void Director::setViewport()
    {
        if (_openGLView)
        {
            _openGLView->setViewPortInPoints(0, 0, _winSizeInPoints.width, _winSizeInPoints.height);
        }
    }

    void Director::setNextDeltaTimeZero(bool nextDeltaTimeZero)
    {
        _nextDeltaTimeZero = nextDeltaTimeZero;
    }

    //
    // FIXME TODO
    // Matrix code MUST NOT be part of the Director
    // MUST BE moved outide.
    // Why the Director must have this code ?
    //
    void Director::initMatrixStack()
    {
        while (!_modelViewMatrixStack.empty())
        {
            _modelViewMatrixStack.pop();
        }

        while (!_projectionMatrixStack.empty())
        {
            _projectionMatrixStack.pop();
        }

        while (!_textureMatrixStack.empty())
        {
            _textureMatrixStack.pop();
        }

        _modelViewMatrixStack.push(MATH::Matrix4::IDENTITY);
        _projectionMatrixStack.push(MATH::Matrix4::IDENTITY);
        _textureMatrixStack.push(MATH::Matrix4::IDENTITY);
    }

    void Director::resetMatrixStack()
    {
        initMatrixStack();
    }

    void Director::popMatrix(MATRIX_STACK_TYPE type)
    {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
        {
            _modelViewMatrixStack.pop();
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
        {
            _projectionMatrixStack.pop();
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
        {
            _textureMatrixStack.pop();
        }
        else
        {
        }
    }

    void Director::loadIdentityMatrix(MATRIX_STACK_TYPE type)
    {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
        {
            _modelViewMatrixStack.top() = MATH::Matrix4::IDENTITY;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
        {
            _projectionMatrixStack.top() = MATH::Matrix4::IDENTITY;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
        {
            _textureMatrixStack.top() = MATH::Matrix4::IDENTITY;
        }
        else
        {
        }
    }

    void Director::loadMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat)
    {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
        {
            _modelViewMatrixStack.top() = mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
        {
            _projectionMatrixStack.top() = mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
        {
            _textureMatrixStack.top() = mat;
        }
        else
        {
        }
    }

    void Director::multiplyMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat)
    {
        if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
        {
            _modelViewMatrixStack.top() *= mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
        {
            _projectionMatrixStack.top() *= mat;
        }
        else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
        {
            _textureMatrixStack.top() *= mat;
        }
        else
        {
        }
    }

    void Director::pushMatrix(MATRIX_STACK_TYPE type)
    {
        if(type == MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW)
        {
            _modelViewMatrixStack.push(_modelViewMatrixStack.top());
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION)
        {
            _projectionMatrixStack.push(_projectionMatrixStack.top());
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE)
        {
            _textureMatrixStack.push(_textureMatrixStack.top());
        }
        else
        {
        }
    }

    const MATH::Matrix4& Director::getMatrix(MATRIX_STACK_TYPE type)
    {
        if(type == MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW)
        {
            return _modelViewMatrixStack.top();
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION)
        {
            return _projectionMatrixStack.top();
        }
        else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE)
        {
            return _textureMatrixStack.top();
        }

        return  _modelViewMatrixStack.top();
    }

    void Director::setProjection(Projection projection)
    {
        MATH::Sizef size = _winSizeInPoints;

        setViewport();

        switch (projection)
        {
            case Projection::_2D:
            {
                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

                MATH::Matrix4 orthoMatrix;
                MATH::Matrix4::createOrthographicOffCenter(0, size.width, 0, size.height, -1024, 1024, &orthoMatrix);
                multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, orthoMatrix);
                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
                break;
            }

            case Projection::_3D:
            {
                float zeye = this->getZEye();

                MATH::Matrix4 matrixPerspective, matrixLookup;

                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

                // issue #1334
                MATH::Matrix4::createPerspective(60, (GLfloat)size.width/size.height, 10, zeye+size.height/2, &matrixPerspective);

                multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, matrixPerspective);

                MATH::Vector3f eye(size.width/2, size.height/2, zeye), center(size.width/2, size.height/2, 0.0f), up(0.0f, 1.0f, 0.0f);
                MATH::Matrix4::createLookAt(eye, center, up, &matrixLookup);
                multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, matrixLookup);

                loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
                break;
            }

            case Projection::CUSTOM:
                // Projection Delegate is no longer needed
                // since the event "PROJECTION CHANGED" is emitted
                break;

            default:
                break;
        }

        _projection = projection;
        setProjectionMatrixDirty();

        _eventDispatcher->dispatchEvent(_eventProjectionChanged);
    }

    void Director::purgeCachedData(void)
    {
        if (s_SharedDirector->getOpenGLView())
        {
            _textureCache->removeUnusedTextures();
        }
    }

    float Director::getZEye(void) const
    {
        return (_winSizeInPoints.height / 1.1566f);
    }

    void Director::setAlphaBlending(bool on)
    {
        if (on)
        {
            blendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            blendFunc(GL_ONE, GL_ZERO);
        }
    }

    void Director::setDepthTest(bool on)
    {
        _renderer->setDepthTest(on);
    }

    void Director::setClearColor(const Color4F& clearColor)
    {
        _renderer->setClearColor(clearColor);
        auto defaultFBO = FrameBuffer::getOrCreateDefaultFBO(_openGLView);

        if(defaultFBO) defaultFBO->setClearColor(clearColor);
    }

    static void GLToClipTransform(MATH::Matrix4 *transformOut)
    {
        if(nullptr == transformOut) return;

        Director* director = Director::getInstance();

        auto projection = director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
        auto modelview = director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        *transformOut = projection * modelview;
    }

    MATH::Vector2f Director::convertToGL(const MATH::Vector2f& uiPoint)
    {
        MATH::Matrix4 transform;
        GLToClipTransform(&transform);

        MATH::Matrix4 transformInv = transform.getInversed();

        // Calculate z=0 using -> transform*[0, 0, 0, 1]/w
        float zClip = transform.m[14]/transform.m[15];

        MATH::Sizef glSize = _openGLView->getDesignResolutionSize();
        MATH::Vector4f clipCoord(2.0f*uiPoint.x/glSize.width - 1.0f, 1.0f - 2.0f*uiPoint.y/glSize.height, zClip, 1);

        MATH::Vector4f glCoord;
        //transformInv.transformPoint(clipCoord, &glCoord);
        transformInv.transformVector(clipCoord, &glCoord);
        float factor = 1.0/glCoord.w;
        return MATH::Vector2f(glCoord.x * factor, glCoord.y * factor);
    }

    MATH::Vector2f Director::convertToUI(const MATH::Vector2f& glPoint)
    {
        MATH::Matrix4 transform;
        GLToClipTransform(&transform);

        MATH::Vector4f clipCoord;
        // Need to calculate the zero depth from the transform.
        MATH::Vector4f glCoord(glPoint.x, glPoint.y, 0.0, 1);
        transform.transformVector(glCoord, &clipCoord);

        /*
        BUG-FIX #5506

        a = (Vx, Vy, Vz, 1)
        b = (a×M)T
        Out = 1 ⁄ bw(bx, by, bz)
        */

        clipCoord.x = clipCoord.x / clipCoord.w;
        clipCoord.y = clipCoord.y / clipCoord.w;
        clipCoord.z = clipCoord.z / clipCoord.w;

        MATH::Sizef glSize = _openGLView->getDesignResolutionSize();
        float factor = 1.0/glCoord.w;
        return MATH::Vector2f(glSize.width*(clipCoord.x*0.5 + 0.5) * factor, glSize.height*(-clipCoord.y*0.5 + 0.5) * factor);
    }

    const MATH::Sizef& Director::getWinSize(void) const
    {
        return _winSizeInPoints;
    }

    MATH::Sizef Director::getWinSizeInPixels() const
    {
        return MATH::Sizef(_winSizeInPoints.width * _contentScaleFactor, _winSizeInPoints.height * _contentScaleFactor);
    }

    MATH::Sizef Director::getVisibleSize() const
    {
        if (_openGLView)
        {
            return _openGLView->getVisibleSize();
        }
        else
        {
            return MATH::SizefZERO;
        }
    }

    MATH::Vector2f Director::getVisibleOrigin() const
    {
        if (_openGLView)
        {
            return _openGLView->getVisibleOrigin();
        }
        else
        {
            return MATH::SizefZERO;
        }
    }

    // scene management

    void Director::runWithScene(Scene *scene)
    {
        pushScene(scene);
        startAnimation();
    }

    void Director::replaceScene(Scene *scene)
    {
        if (_runningScene == nullptr) {
            runWithScene(scene);
            return;
        }

        if (scene == _nextScene)
            return;

        if (_nextScene)
        {
            if (_nextScene->isRunning())
            {
                _nextScene->onExit();
            }
            _nextScene->cleanup();
            _nextScene = nullptr;
        }

        ssize_t index = _scenesStack.size();

        _sendCleanupToScene = true;
        _scenesStack.replace(index - 1, scene);

        _nextScene = scene;
    }

    void Director::pushScene(Scene *scene)
    {
        _sendCleanupToScene = false;

        _scenesStack.pushBack(scene);
        _nextScene = scene;
    }

    void Director::popScene(void)
    {
        _scenesStack.popBack();
        ssize_t c = _scenesStack.size();

        if (c == 0)
        {
            end();
        }
        else
        {
            _sendCleanupToScene = true;
            _nextScene = _scenesStack.at(c - 1);
        }
    }

    void Director::popToRootScene(void)
    {
        popToSceneStackLevel(1);
    }

    void Director::popToSceneStackLevel(int level)
    {
        ssize_t c = _scenesStack.size();

        // level 0? -> end
        if (level == 0)
        {
            end();
            return;
        }

        // current level or lower -> nothing
        if (level >= c)
            return;

        auto fisrtOnStackScene = _scenesStack.back();
        if (fisrtOnStackScene == _runningScene)
        {
            _scenesStack.popBack();
            --c;
        }

        // pop stack until reaching desired level
        while (c > level)
        {
            auto current = _scenesStack.back();

            if (current->isRunning())
            {
                current->onExit();
            }

            current->cleanup();
            _scenesStack.popBack();
            --c;
        }

        _nextScene = _scenesStack.back();

        // cleanup running scene
        _sendCleanupToScene = true;
    }

    void Director::end()
    {
        _purgeDirectorInNextLoop = true;
    }

    void Director::restart()
    {
        _restartDirectorInNextLoop = true;
    }

    void Director::reset()
    {
        if (_runningScene)
        {
            _runningScene->onExit();
            _runningScene->cleanup();
            _runningScene->release();
        }

        _runningScene = nullptr;
        _nextScene = nullptr;

        // cleanup scheduler
        getScheduler()->unscheduleAll();

        // Remove all events
        if (_eventDispatcher)
        {
            _eventDispatcher->removeAllEventListeners();
        }

        // remove all objects, but don't release it.
        // runWithScene might be executed after 'end'.
        _scenesStack.clear();

        stopAnimation();

        SAFE_RELEASE_NULL(_notificationNode);

        // purge all managed caches
        GLProgramCache::destroyInstance();
        GLProgramStateCache::destroyInstance();

        invalidateStateCache();

        RenderState::finalize();

        destroyTextureCache();
    }

    void Director::purgeDirector()
    {
        reset();

        // OpenGL view
        if (_openGLView)
        {
            _openGLView->end();
            _openGLView = nullptr;
        }

        // delete Director
        release();
    }

    void Director::restartDirector()
    {
        reset();

        // RenderState need to be reinitialized
        RenderState::initialize();

        // Texture cache need to be reinitialized
        initTextureCache();

        // Reschedule for action manager
        getScheduler()->scheduleUpdate(getActionManager(), Scheduler::PRIORITY_SYSTEM, false);

        // release the objects
        PoolManager::getInstance().getCurrentPool()->clear();
    }

    void Director::setNextScene()
    {
        // If it is not a transition, call onExit/cleanup
         if (_runningScene)
         {
             _runningScene->onExitTransitionDidStart();
             _runningScene->onExit();
         }

         // issue #709. the root node (scene) should receive the cleanup message too
         // otherwise it might be leaked.
         if (_sendCleanupToScene && _runningScene)
         {
             _runningScene->cleanup();
         }

        if (_runningScene)
        {
            _runningScene->release();
        }
        _runningScene = _nextScene;
        _nextScene->retain();
        _nextScene = nullptr;

        if (_runningScene)
        {
            _runningScene->onEnter();
            _runningScene->onEnterTransitionDidFinish();
        }
    }

    void Director::pause()
    {
        if (_paused)
        {
            return;
        }

        _oldAnimationInterval = _animationInterval;

        // when paused, don't consume CPU
        setAnimationInterval(1 / 4.0);
        _paused = true;
    }

    void Director::resume()
    {
        if (! _paused)
        {
            return;
        }

        setAnimationInterval(_oldAnimationInterval);

        _paused = false;
        _deltaTime = 0;
        // fix issue #3509, skip one fps to avoid incorrect time calculation.
        setNextDeltaTimeZero(true);
    }

    void Director::setContentScaleFactor(float scaleFactor)
    {
        if (scaleFactor != _contentScaleFactor)
        {
            _contentScaleFactor = scaleFactor;
            _isStatusLabelUpdated = true;
        }
    }

    void Director::setNotificationNode(Node *node)
    {
        SAFE_RELEASE(_notificationNode);
        _notificationNode = node;
        SAFE_RETAIN(_notificationNode);
    }

    void Director::setScheduler(Scheduler* scheduler)
    {
        if (_scheduler != scheduler)
        {
            SAFE_RETAIN(scheduler);
            SAFE_RELEASE(_scheduler);
            _scheduler = scheduler;
        }
    }

    void Director::setActionManager(ActionManager* actionManager)
    {
        if (_actionManager != actionManager)
        {
            SAFE_RETAIN(actionManager);
            SAFE_RELEASE(_actionManager);
            _actionManager = actionManager;
        }
    }

    void Director::setEventDispatcher(EventDispatcher* dispatcher)
    {
        if (_eventDispatcher != dispatcher)
        {
            SAFE_RETAIN(dispatcher);
            SAFE_RELEASE(_eventDispatcher);
            _eventDispatcher = dispatcher;
        }
    }

    /***************************************************
    * implementation of DisplayLinkDirector
    **************************************************/

    // should we implement 4 types of director ??
    // I think DisplayLinkDirector is enough
    // so we now only support DisplayLinkDirector
    void DisplayLinkDirector::startAnimation()
    {
        _invalid = false;

        // fix issue #3509, skip one fps to avoid incorrect time calculation.
        setNextDeltaTimeZero(true);
    }

    void DisplayLinkDirector::mainLoop()
    {
        if (_purgeDirectorInNextLoop)
        {
            _purgeDirectorInNextLoop = false;
            purgeDirector();
        }
        else if (_restartDirectorInNextLoop)
        {
            _restartDirectorInNextLoop = false;
            restartDirector();
        }
        else if (! _invalid)
        {
            drawScene();

            // release the objects
            PoolManager::getInstance().getCurrentPool()->clear();
        }
    }

    void DisplayLinkDirector::stopAnimation()
    {
        _invalid = true;
    }

    void DisplayLinkDirector::setAnimationInterval(float interval)
    {
        _animationInterval = interval;
        if (! _invalid)
        {
            stopAnimation();
            startAnimation();
        }
    }
}
