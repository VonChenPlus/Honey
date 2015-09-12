#ifndef DIRECTOR_H
#define DIRECTOR_H

#include <stack>
#include "BASE/HObject.h"
#include "MATH/Matrix.h"

namespace GRAPH
{
    class TextureCache;
    class GLView;

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

        TextureCache* getTextureCache() const;

        void pushMatrix(MATRIX_STACK_TYPE type);
        void popMatrix(MATRIX_STACK_TYPE type);
        void loadIdentityMatrix(MATRIX_STACK_TYPE type);
        void loadMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        void multiplyMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        const MATH::Matrix4& getMatrix(MATRIX_STACK_TYPE type);
        void resetMatrixStack();

        MATH::Vector2f convertToGL(const MATH::Vector2f& point);

    protected:
        void initMatrixStack();

        void glToClipTransform(MATH::Matrix4 *transformOut);

    private:
        std::stack<MATH::Matrix4> modelViewMatrixStack_;
        std::stack<MATH::Matrix4> projectionMatrixStack_;
        std::stack<MATH::Matrix4> textureMatrixStack_;
        //texture cache belongs to this director
        TextureCache *textureCache_;
        GLView *glView_;
    };
}

#endif // DIRECTOR_H
