#ifndef DIRECTOR_H
#define DIRECTOR_H

#include <stack>
#include "BASE/HObject.h"
#include "MATH/Matrix.h"

namespace GRAPH
{
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

        void pushMatrix(MATRIX_STACK_TYPE type);
        void popMatrix(MATRIX_STACK_TYPE type);
        void loadIdentityMatrix(MATRIX_STACK_TYPE type);
        void loadMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        void multiplyMatrix(MATRIX_STACK_TYPE type, const MATH::Matrix4& mat);
        const MATH::Matrix4& getMatrix(MATRIX_STACK_TYPE type);
        void resetMatrixStack();

    protected:
        void initMatrixStack();

    private:
        std::stack<MATH::Matrix4> modelViewMatrixStack_;
        std::stack<MATH::Matrix4> projectionMatrixStack_;
        std::stack<MATH::Matrix4> textureMatrixStack_;
    };
}

#endif // DIRECTOR_H
