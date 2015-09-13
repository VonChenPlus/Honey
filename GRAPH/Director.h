#ifndef DIRECTOR_H
#define DIRECTOR_H

#include <stack>
#include "BASE/HObject.h"
#include "MATH/Matrix.h"
#include "MATH/Size.h"

namespace GRAPH
{
    class GLView;
    class ActionManager;
    class Scheduler;
    class EventDispatcher;
    class Renderer;
    class TextureCache;

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

        ActionManager* getActionManager() const { return actionManager_; }
        Scheduler *getScheduler() const { return scheduler_; }
        EventDispatcher *getEventDispatcher() const { return eventDispatcher_; }
        TextureCache* getTextureCache() const { return textureCache_; }
        Renderer* getRenderer() const { return renderer_; }

        void pushMatrix(MATRIX_STACK_TYPE type);
        void popMatrix(MATRIX_STACK_TYPE type);
        void loadIdentityMatrix(MATRIX_STACK_TYPE type);
        void loadMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        void multiplyMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        const MATH::Matrix4& getMatrix(MATRIX_STACK_TYPE type);
        void resetMatrixStack();

        const MATH::Sizef& getWinSize() const;

        MATH::Vector2f convertToGL(const MATH::Vector2f& point);
        MATH::Vector2f convertToUI(const MATH::Vector2f& point);

    protected:
        void initMatrixStack();

        void glToClipTransform(MATH::Matrix4 *transformOut);

    private:
        std::stack<MATH::Matrix4> modelViewMatrixStack_;
        std::stack<MATH::Matrix4> projectionMatrixStack_;
        std::stack<MATH::Matrix4> textureMatrixStack_;

        ActionManager *actionManager_;
        Scheduler *scheduler_;
        EventDispatcher *eventDispatcher_;
        GLView *glView_;
        TextureCache *textureCache_;
        Renderer *renderer_;
    };
}

#endif // DIRECTOR_H
