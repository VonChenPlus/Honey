#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include <string>
#include <functional>
#include <cstdint>
#include "BASE/HObject.h"
#include "GRAPH/RENDERER/GLCommon.h"
#include "GRAPH/Types.h"

namespace GRAPH
{
    class RenderState : public HObject
    {
    protected:
        RenderState();
        ~RenderState();

    public:
        enum Blend
        {
            BLEND_ZERO = GL_ZERO,
            BLEND_ONE = GL_ONE,
            BLEND_SRC_COLOR = GL_SRC_COLOR,
            BLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
            BLEND_DST_COLOR = GL_DST_COLOR,
            BLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
            BLEND_SRC_ALPHA = GL_SRC_ALPHA,
            BLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
            BLEND_DST_ALPHA = GL_DST_ALPHA,
            BLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
            BLEND_CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
            BLEND_ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
            BLEND_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
        };

        enum DepthFunction
        {
            DEPTH_NEVER = GL_NEVER,
            DEPTH_LESS = GL_LESS,
            DEPTH_EQUAL = GL_EQUAL,
            DEPTH_LEQUAL = GL_LEQUAL,
            DEPTH_GREATER = GL_GREATER,
            DEPTH_NOTEQUAL = GL_NOTEQUAL,
            DEPTH_GEQUAL = GL_GEQUAL,
            DEPTH_ALWAYS = GL_ALWAYS
        };

        enum CullFaceSide
        {
            CULL_FACE_SIDE_BACK = GL_BACK,
            CULL_FACE_SIDE_FRONT = GL_FRONT,
            CULL_FACE_SIDE_FRONT_AND_BACK = GL_FRONT_AND_BACK
        };

        enum FrontFace
        {
            FRONT_FACE_CW = GL_CW,
            FRONT_FACE_CCW = GL_CCW
        };

        enum StencilFunction
        {
            STENCIL_NEVER = GL_NEVER,
            STENCIL_ALWAYS = GL_ALWAYS,
            STENCIL_LESS = GL_LESS,
            STENCIL_LEQUAL = GL_LEQUAL,
            STENCIL_EQUAL = GL_EQUAL,
            STENCIL_GREATER = GL_GREATER,
            STENCIL_GEQUAL = GL_GEQUAL,
            STENCIL_NOTEQUAL = GL_NOTEQUAL
        };

        enum StencilOperation
        {
            STENCIL_OP_KEEP = GL_KEEP,
            STENCIL_OP_ZERO = GL_ZERO,
            STENCIL_OP_REPLACE = GL_REPLACE,
            STENCIL_OP_INCR = GL_INCR,
            STENCIL_OP_DECR = GL_DECR,
            STENCIL_OP_INVERT = GL_INVERT,
            STENCIL_OP_INCR_WRAP = GL_INCR_WRAP,
            STENCIL_OP_DECR_WRAP = GL_DECR_WRAP
        };

        class StateBlock : public HObject
        {
            friend class RenderState;
            friend class Pass;
            friend class RenderQueue;
            friend class Renderer;

        public:
            static StateBlock* create();

            StateBlock();
            ~StateBlock();

            void bind();

            void setBlendFunc(const BlendFunc& blendFunc);
            void setBlend(bool enabled);
            void setBlendSrc(Blend blend);
            void setBlendDst(Blend blend);

            void setCullFace(bool enabled);
            void setCullFaceSide(CullFaceSide side);
            void setFrontFace(FrontFace winding);

            void setDepthTest(bool enabled);
            void setDepthWrite(bool enabled);
            void setDepthFunction(DepthFunction func);

            void setState(const std::string& name, const std::string& value);

            enum
            {
                RS_BLEND = (1 << 0),
                RS_BLEND_FUNC = (1 << 1),
                RS_CULL_FACE = (1 << 2),
                RS_DEPTH_TEST = (1 << 3),
                RS_DEPTH_WRITE = (1 << 4),
                RS_DEPTH_FUNC = (1 << 5),
                RS_CULL_FACE_SIDE = (1 << 6),
                RS_FRONT_FACE = (1 << 11),

                RS_ALL_ONES = 0xFFFFFFFF,
            };

            void invalidate(long stateBits);
            void restore(long stateOverrideBits);

        protected:
            void bindNoRestore();
            void enableDepthWrite();

            void cloneInto(StateBlock* renderState) const;

            bool cullFaceEnabled_;
            bool depthTestEnabled_;
            bool depthWriteEnabled_;
            DepthFunction depthFunction_;
            bool blendEnabled_;
            Blend blendSrc_;
            Blend blendDst_;
            CullFaceSide cullFaceSide_;
            FrontFace frontFace_;
            bool stencilTestEnabled_;
            unsigned int stencilWrite_;
            StencilFunction stencilFunction_;
            int stencilFunctionRef_;
            unsigned int stencilFunctionMask_;
            StencilOperation stencilOpSfail_;
            StencilOperation stencilOpDpfail_;
            StencilOperation stencilOpDppass_;
            long stateBits_;
        };

    public:
        StateBlock* getStateBlock() const;

        static StateBlock& DefaultState();

    private:
        mutable StateBlock* stateBlock_;
    };
}

#endif // RENDERSTATE_H
