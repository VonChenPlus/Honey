#ifndef DIRECTOR_H
#define DIRECTOR_H

#include <stack>
#include "BASE/HObject.h"
#include "MATH/Matrix.h"
#include "MATH/Size.h"

namespace GRAPH
{
    class RenderView;
    class ActionManager;
    class Scheduler;
    class EventDispatcher;
    class Renderer;
    class TextureCache;
    class Scene;
    class Camera;

    enum class Projection
    {
        /// Sets a 2D projection (orthogonal projection).
        _2D,

        /// Sets a 3D projection with a fovy=60, znear=0.5f and zfar=1500.
        _3D,

        /// Default projection is 3D projection.
        DEFAULT = _3D,
    };

    enum class MATRIX_STACK_TYPE
    {
        /// Model view matrix stack
        MATRIX_STACK_MODELVIEW,

        /// projection matrix stack
        MATRIX_STACK_PROJECTION,

        /// texture matrix stack
        MATRIX_STACK_TEXTURE
    };

    class Director : public HObject
    {
    public:
        static Director& getInstance();

        Director();
        virtual ~Director();

        virtual bool init();

        inline Camera* getCamera() { return camera_; }
        inline Scene* getRunningScene() { return runningScene_; }

        inline RenderView *getRenderView() { return renderView_; }
        void setRenderView(RenderView *view);

        ActionManager* getActionManager() const { return actionManager_; }
        Scheduler *getScheduler() const { return scheduler_; }
        EventDispatcher *getEventDispatcher() const { return eventDispatcher_; }
        TextureCache* getTextureCache() const { return textureCache_; }
        Renderer* getRenderer() const { return renderer_; }

        void setGLDefaultValues();

        inline Projection getProjection() { return projection_; }
        void setProjection(Projection projection);

        void setViewPort();

        void setAlphaBlending(bool on);
        void setDepthTest(bool on);

        void pushMatrix(MATRIX_STACK_TYPE type);
        void popMatrix(MATRIX_STACK_TYPE type);
        void loadIdentityMatrix(MATRIX_STACK_TYPE type);
        void loadMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        void multiplyMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        const MATH::Matrix4& getMatrix(MATRIX_STACK_TYPE type);
        void resetMatrixStack();

        const MATH::Sizef& getWinSize() const;
        float getZEye(void) const;

        void runWithScene(Scene *scene);
        void pushScene(Scene *scene);
        void popScene();

        MATH::Vector2f convertToGL(const MATH::Vector2f& point);
        MATH::Vector2f convertToUI(const MATH::Vector2f& point);

        bool checkVisibility(const MATH::Matrix4 &transform, const MATH::Sizef &size);

        void mainLoop();
        void drawScene();

    protected:
        void setNextScene();

        void initMatrixStack();

        void glToClipTransform(MATH::Matrix4 *transformOut);

    private:
        bool paused_;

        Camera *camera_;

        Scene *runningScene_;
        Scene *nextScene_;
        HObjectVector<Scene*> scenesStack_;

        Projection projection_;

        std::stack<MATH::Matrix4> modelViewMatrixStack_;
        std::stack<MATH::Matrix4> projectionMatrixStack_;
        std::stack<MATH::Matrix4> textureMatrixStack_;

        ActionManager *actionManager_;
        Scheduler *scheduler_;
        EventDispatcher *eventDispatcher_;
        RenderView *renderView_;
        TextureCache *textureCache_;
        Renderer *renderer_;
    };
}

#endif // DIRECTOR_H
