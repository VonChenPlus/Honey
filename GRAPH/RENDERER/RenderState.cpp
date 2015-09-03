#include <string>
#include "GRAPH/RENDERER/RenderState.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
    RenderState::RenderState() {
        stateBlock_ = StateBlock::create();
        SAFE_RETAIN(stateBlock_);
    }

    RenderState::~RenderState() {
        SAFE_RELEASE(stateBlock_);
    }

    RenderState::StateBlock* RenderState::getStateBlock() const {
        return stateBlock_;
    }

    RenderState::StateBlock &RenderState::DefaultState() {
        static RenderState::StateBlock instance;
        return instance;
    }

    RenderState::StateBlock* RenderState::StateBlock::create()
    {
        auto state = new (std::nothrow) RenderState::StateBlock();
        if (state) {
            state->autorelease();
        }

        return state;
    }

    RenderState::StateBlock::StateBlock()
        : cullFaceEnabled_(false)
        , depthTestEnabled_(true), depthWriteEnabled_(false), depthFunction_(RenderState::DEPTH_LESS)
        , blendEnabled_(true), blendSrc_(RenderState::BLEND_ONE), blendDst_(RenderState::BLEND_ZERO)
        , cullFaceSide_(CULL_FACE_SIDE_BACK), frontFace_(FRONT_FACE_CCW)
        , stencilTestEnabled_(false), stencilWrite_(RS_ALL_ONES)
        , stencilFunction_(RenderState::STENCIL_ALWAYS), stencilFunctionRef_(0), stencilFunctionMask_(RS_ALL_ONES)
        , stencilOpSfail_(RenderState::STENCIL_OP_KEEP), stencilOpDpfail_(RenderState::STENCIL_OP_KEEP), stencilOpDppass_(RenderState::STENCIL_OP_KEEP)
        , stateBits_(0L) {
    }

    RenderState::StateBlock::~StateBlock() {
    }

    void RenderState::StateBlock::bind() {
        // When the public bind() is called with no RenderState object passed in,
        // we assume we are being called to bind the state of a single StateBlock,
        // irrespective of whether it belongs to a hierarchy of RenderStates.
        // Therefore, we call restore() here with only this StateBlock's override
        // bits to restore state before applying the new state.
        StateBlock::restore(stateBits_);

        bindNoRestore();
    }

    void RenderState::StateBlock::bindNoRestore() {
        // Update any state that differs from _defaultState and flip _defaultState bits
        if ((stateBits_ & RS_BLEND) && (blendEnabled_ != blendEnabled_)) {
            if (blendEnabled_)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
            blendEnabled_ = blendEnabled_;
        }
        if ((stateBits_ & RS_BLEND_FUNC) && (blendSrc_ != blendSrc_ || blendDst_ != blendDst_)) {
            GLStateCache::BlendFunc((GLenum)blendSrc_, (GLenum)blendDst_);
            blendSrc_ = blendSrc_;
            blendDst_ = blendDst_;
        }
        if ((stateBits_ & RS_CULL_FACE) && (cullFaceEnabled_ != cullFaceEnabled_)) {
            if (cullFaceEnabled_)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            cullFaceEnabled_ = cullFaceEnabled_;
        }
        if ((stateBits_ & RS_CULL_FACE_SIDE) && (cullFaceSide_ != cullFaceSide_)) {
            glCullFace((GLenum)cullFaceSide_);
            cullFaceSide_ = cullFaceSide_;
        }
        if ((stateBits_ & RS_FRONT_FACE) && (frontFace_ != frontFace_)) {
            glFrontFace((GLenum)frontFace_);
            frontFace_ = frontFace_;
        }
        if ((stateBits_ & RS_DEPTH_TEST) && (depthTestEnabled_ != depthTestEnabled_)) {
            if (depthTestEnabled_)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
            depthTestEnabled_ = depthTestEnabled_;
        }
        if ((stateBits_ & RS_DEPTH_WRITE) && (depthWriteEnabled_ != depthWriteEnabled_)) {
            glDepthMask(depthWriteEnabled_ ? GL_TRUE : GL_FALSE);
            depthWriteEnabled_ = depthWriteEnabled_;
        }
        if ((stateBits_ & RS_DEPTH_FUNC) && (depthFunction_ != depthFunction_)) {
            glDepthFunc((GLenum)depthFunction_);
            depthFunction_ = depthFunction_;
        }

        stateBits_ |= stateBits_;
    }

    void RenderState::StateBlock::restore(long stateOverrideBits) {
        // If there is no state to restore (i.e. no non-default state), do nothing.
        if ( (stateOverrideBits | stateBits_) == stateOverrideBits) {
            return;
        }

        // Restore any state that is not overridden and is not default
        if (!(stateOverrideBits & RS_BLEND) && (stateBits_ & RS_BLEND)) {
            glEnable(GL_BLEND);
            stateBits_ &= ~RS_BLEND;
            blendEnabled_ = true;
        }
        if (!(stateOverrideBits & RS_BLEND_FUNC) && (stateBits_ & RS_BLEND_FUNC)) {
            GLStateCache::BlendFunc(GL_ONE, GL_ZERO);
            stateBits_ &= ~RS_BLEND_FUNC;
            blendSrc_ = RenderState::BLEND_ONE;
            blendDst_ = RenderState::BLEND_ZERO;
        }
        if (!(stateOverrideBits & RS_CULL_FACE) && (stateBits_ & RS_CULL_FACE)) {
            glDisable(GL_CULL_FACE);
            stateBits_ &= ~RS_CULL_FACE;
            cullFaceEnabled_ = false;
        }
        if (!(stateOverrideBits & RS_CULL_FACE_SIDE) && (stateBits_ & RS_CULL_FACE_SIDE)) {
            glCullFace((GLenum)GL_BACK);
            stateBits_ &= ~RS_CULL_FACE_SIDE;
            cullFaceSide_ = RenderState::CULL_FACE_SIDE_BACK;
        }
        if (!(stateOverrideBits & RS_FRONT_FACE) && (stateBits_ & RS_FRONT_FACE)) {
            glFrontFace((GLenum)GL_CCW);
            stateBits_ &= ~RS_FRONT_FACE;
            frontFace_ = RenderState::FRONT_FACE_CCW;
        }
        if (!(stateOverrideBits & RS_DEPTH_TEST) && (stateBits_ & RS_DEPTH_TEST)) {
            glEnable(GL_DEPTH_TEST);
            stateBits_ &= ~RS_DEPTH_TEST;
            depthTestEnabled_ = true;
        }
        if (!(stateOverrideBits & RS_DEPTH_WRITE) && (stateBits_ & RS_DEPTH_WRITE)) {
            glDepthMask(GL_FALSE);
            stateBits_ &= ~RS_DEPTH_WRITE;
            depthWriteEnabled_ = false;
        }
        if (!(stateOverrideBits & RS_DEPTH_FUNC) && (stateBits_ & RS_DEPTH_FUNC)) {
            glDepthFunc((GLenum)GL_LESS);
            stateBits_ &= ~RS_DEPTH_FUNC;
            depthFunction_ = RenderState::DEPTH_LESS;
        }
    }

    void RenderState::StateBlock::enableDepthWrite() {
        // Internal method used to restore depth writing before a
        // clear operation. This is necessary if the last code to draw before the
        // next frame leaves depth writing disabled.
        if (!depthWriteEnabled_) {
            glDepthMask(GL_TRUE);
            stateBits_ &= ~RS_DEPTH_WRITE;
            depthWriteEnabled_ = true;
        }
    }

    void RenderState::StateBlock::cloneInto(StateBlock* state) const {
        state->cullFaceEnabled_ = cullFaceEnabled_;
        state->depthTestEnabled_ = depthTestEnabled_;
        state->depthWriteEnabled_ = depthWriteEnabled_;
        state->depthFunction_ = depthFunction_;
        state->blendEnabled_ = blendEnabled_;
        state->blendSrc_ = blendSrc_;
        state->blendDst_ = blendDst_;
        state->cullFaceSide_ = cullFaceSide_;
        state->frontFace_ = frontFace_;
        state->stencilTestEnabled_ = stencilTestEnabled_;
        state->stencilWrite_ = stencilWrite_;
        state->stencilFunction_ = stencilFunction_;
        state->stencilFunctionRef_ = stencilFunctionRef_;
        state->stencilFunctionMask_ = stencilFunctionMask_;
        state->stencilOpSfail_ = stencilOpSfail_;
        state->stencilOpDpfail_ = stencilOpDpfail_;
        state->stencilOpDppass_ = stencilOpDppass_;
        state->stateBits_ = stateBits_;
    }

    static RenderState::Blend parseBlend(const std::string& value) {
        // Convert the string to uppercase for comparison.
        std::string upper(value);
        std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
        if (upper == "ZERO")
            return RenderState::BLEND_ZERO;
        else if (upper == "ONE")
            return RenderState::BLEND_ONE;
        else if (upper == "SRC_COLOR")
            return RenderState::BLEND_SRC_COLOR;
        else if (upper == "ONE_MINUS_SRC_COLOR")
            return RenderState::BLEND_ONE_MINUS_SRC_COLOR;
        else if (upper == "DST_COLOR")
            return RenderState::BLEND_DST_COLOR;
        else if (upper == "ONE_MINUS_DST_COLOR")
            return RenderState::BLEND_ONE_MINUS_DST_COLOR;
        else if (upper == "SRC_ALPHA")
            return RenderState::BLEND_SRC_ALPHA;
        else if (upper == "ONE_MINUS_SRC_ALPHA")
            return RenderState::BLEND_ONE_MINUS_SRC_ALPHA;
        else if (upper == "DST_ALPHA")
            return RenderState::BLEND_DST_ALPHA;
        else if (upper == "ONE_MINUS_DST_ALPHA")
            return RenderState::BLEND_ONE_MINUS_DST_ALPHA;
        else if (upper == "CONSTANT_ALPHA")
            return RenderState::BLEND_CONSTANT_ALPHA;
        else if (upper == "ONE_MINUS_CONSTANT_ALPHA")
            return RenderState::BLEND_ONE_MINUS_CONSTANT_ALPHA;
        else if (upper == "SRC_ALPHA_SATURATE")
            return RenderState::BLEND_SRC_ALPHA_SATURATE;
        else {
            throw _HException_Normal("Unsupported blend value");
        }
    }

    static RenderState::DepthFunction parseDepthFunc(const std::string& value) {
        // Convert string to uppercase for comparison
        std::string upper(value);
        std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
        if (upper == "NEVER")
            return RenderState::DEPTH_NEVER;
        else if (upper == "LESS")
            return RenderState::DEPTH_LESS;
        else if (upper == "EQUAL")
            return RenderState::DEPTH_EQUAL;
        else if (upper == "LEQUAL")
            return RenderState::DEPTH_LEQUAL;
        else if (upper == "GREATER")
            return RenderState::DEPTH_GREATER;
        else if (upper == "NOTEQUAL")
            return RenderState::DEPTH_NOTEQUAL;
        else if (upper == "GEQUAL")
            return RenderState::DEPTH_GEQUAL;
        else if (upper == "ALWAYS")
            return RenderState::DEPTH_ALWAYS;
        else {
            throw _HException_Normal("Unsupported depth function value");
        }
    }

    static RenderState::CullFaceSide parseCullFaceSide(const std::string& value) {
        // Convert string to uppercase for comparison
        std::string upper(value);
        std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
        if (upper == "BACK")
            return RenderState::CULL_FACE_SIDE_BACK;
        else if (upper == "FRONT")
            return RenderState::CULL_FACE_SIDE_FRONT;
        else if (upper == "FRONT_AND_BACK")
            return RenderState::CULL_FACE_SIDE_FRONT_AND_BACK;
        else {
            throw _HException_Normal("Unsupported cull face side value");
        }
    }

    static RenderState::FrontFace parseFrontFace(const std::string& value) {
        // Convert string to uppercase for comparison
        std::string upper(value);
        std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
        if (upper == "CCW")
            return RenderState::FRONT_FACE_CCW;
        else if (upper == "CW")
            return RenderState::FRONT_FACE_CW;
        else {
            throw _HException_Normal("Unsupported front face side value");
        }
    }

    static RenderState::StencilFunction parseStencilFunc(const std::string& value) {
        // Convert string to uppercase for comparison
        std::string upper(value);
        std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
        if (upper == "NEVER")
            return RenderState::STENCIL_NEVER;
        else if (upper == "LESS")
            return RenderState::STENCIL_LESS;
        else if (upper == "EQUAL")
            return RenderState::STENCIL_EQUAL;
        else if (upper == "LEQUAL")
            return RenderState::STENCIL_LEQUAL;
        else if (upper == "GREATER")
            return RenderState::STENCIL_GREATER;
        else if (upper == "NOTEQUAL")
            return RenderState::STENCIL_NOTEQUAL;
        else if (upper == "GEQUAL")
            return RenderState::STENCIL_GEQUAL;
        else if (upper == "ALWAYS")
            return RenderState::STENCIL_ALWAYS;
        else {
            throw _HException_Normal("Unsupported stencil function value");
        }
    }

    static RenderState::StencilOperation parseStencilOp(const std::string& value) {
        // Convert string to uppercase for comparison
        std::string upper(value);
        std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
        if (upper == "KEEP")
            return RenderState::STENCIL_OP_KEEP;
        else if (upper == "ZERO")
            return RenderState::STENCIL_OP_ZERO;
        else if (upper == "REPLACE")
            return RenderState::STENCIL_OP_REPLACE;
        else if (upper == "INCR")
            return RenderState::STENCIL_OP_INCR;
        else if (upper == "DECR")
            return RenderState::STENCIL_OP_DECR;
        else if (upper == "INVERT")
            return RenderState::STENCIL_OP_INVERT;
        else if (upper == "INCR_WRAP")
            return RenderState::STENCIL_OP_INCR_WRAP;
        else if (upper == "DECR_WRAP")
            return RenderState::STENCIL_OP_DECR_WRAP;
        else {
            throw _HException_Normal("Unsupported stencil operation value");
        }
    }

    void RenderState::StateBlock::setState(const std::string& name, const std::string& value) {
        if (name.compare("blend") == 0) {
            setBlend(UTILS::STRING::ParseBoolean(value));
        }
        else if (name.compare("blendSrc") == 0) {
            setBlendSrc(parseBlend(value));
        }
        else if (name.compare("blendDst") == 0) {
            setBlendDst(parseBlend(value));
        }
        else if (name.compare("cullFace") == 0) {
            setCullFace(UTILS::STRING::ParseBoolean(value));
        }
        else if (name.compare("cullFaceSide") == 0) {
            setCullFaceSide(parseCullFaceSide(value));
        }
        else if (name.compare("frontFace") == 0) {
            setFrontFace(parseFrontFace(value));
        }
        else if (name.compare("depthTest") == 0) {
            setDepthTest(UTILS::STRING::ParseBoolean(value));
        }
        else if (name.compare("depthWrite") == 0) {
            setDepthWrite(UTILS::STRING::ParseBoolean(value));
        }
        else if (name.compare("depthFunc") == 0) {
            setDepthFunction(parseDepthFunc(value));
        }
    }

    void RenderState::StateBlock::invalidate(long stateBits) {
        stateBits_ = stateBits;
        restore(0);
    }

    void RenderState::StateBlock::setBlend(bool enabled) {
        blendEnabled_ = enabled;
        if (enabled)
        {
            stateBits_ &= ~RS_BLEND;
        }
        else
        {
            stateBits_ |= RS_BLEND;
        }
    }

    void RenderState::StateBlock::setBlendFunc(const BlendFunc& blendFunc) {
        setBlendSrc((RenderState::Blend)blendFunc.src);
        setBlendDst((RenderState::Blend)blendFunc.dst);
    }

    void RenderState::StateBlock::setBlendSrc(Blend blend) {
        blendSrc_ = blend;
        if (blendSrc_ == BLEND_ONE && blendDst_ == BLEND_ZERO) {
            // Default blend func
            stateBits_ &= ~RS_BLEND_FUNC;
        }
        else {
            stateBits_ |= RS_BLEND_FUNC;
        }
    }

    void RenderState::StateBlock::setBlendDst(Blend blend) {
        blendDst_ = blend;
        if (blendSrc_ == BLEND_ONE && blendDst_ == BLEND_ZERO) {
            // Default blend func
            stateBits_ &= ~RS_BLEND_FUNC;
        }
        else {
            stateBits_ |= RS_BLEND_FUNC;
        }
    }

    void RenderState::StateBlock::setCullFace(bool enabled) {
        cullFaceEnabled_ = enabled;
        if (!enabled) {
            stateBits_ &= ~RS_CULL_FACE;
        }
        else {
            stateBits_ |= RS_CULL_FACE;
        }
    }

    void RenderState::StateBlock::setCullFaceSide(CullFaceSide side) {
        cullFaceSide_ = side;
        if (cullFaceSide_ == CULL_FACE_SIDE_BACK) {
            // Default cull side
            stateBits_ &= ~RS_CULL_FACE_SIDE;
        }
        else {
            stateBits_ |= RS_CULL_FACE_SIDE;
        }
    }

    void RenderState::StateBlock::setFrontFace(FrontFace winding) {
        frontFace_ = winding;
        if (frontFace_ == FRONT_FACE_CCW) {
            // Default front face
            stateBits_ &= ~RS_FRONT_FACE;
        }
        else {
            stateBits_ |= RS_FRONT_FACE;
        }
    }

    void RenderState::StateBlock::setDepthTest(bool enabled) {
        depthTestEnabled_ = enabled;
        if (enabled) {
            stateBits_ &= ~RS_DEPTH_TEST;
        }
        else {
            stateBits_ |= RS_DEPTH_TEST;
        }
    }

    void RenderState::StateBlock::setDepthWrite(bool enabled) {
        depthWriteEnabled_ = enabled;
        if (!enabled) {
            stateBits_ &= ~RS_DEPTH_WRITE;
        }
        else {
            stateBits_ |= RS_DEPTH_WRITE;
        }
    }

    void RenderState::StateBlock::setDepthFunction(DepthFunction func) {
        depthFunction_ = func;
        if (depthFunction_ == DEPTH_LESS) {
            // Default depth function
            stateBits_ &= ~RS_DEPTH_FUNC;
        }
        else {
            stateBits_ |= RS_DEPTH_FUNC;
        }
    }
}
