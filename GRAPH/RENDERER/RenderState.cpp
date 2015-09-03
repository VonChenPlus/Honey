#include <string>
#include "GRAPH/RENDERER/RenderState.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
    RenderState::StateBlock* RenderState::StateBlock::_defaultState = nullptr;

    RenderState::RenderState() {
        _state = StateBlock::create();
        SAFE_RETAIN(_state);
    }

    RenderState::~RenderState() {
        SAFE_RELEASE(_state);
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
        : _cullFaceEnabled(false)
        , _depthTestEnabled(true), _depthWriteEnabled(false), _depthFunction(RenderState::DEPTH_LESS)
        , _blendEnabled(true), _blendSrc(RenderState::BLEND_ONE), _blendDst(RenderState::BLEND_ZERO)
        , _cullFaceSide(CULL_FACE_SIDE_BACK), _frontFace(FRONT_FACE_CCW)
        , _stencilTestEnabled(false), _stencilWrite(RS_ALL_ONES)
        , _stencilFunction(RenderState::STENCIL_ALWAYS), _stencilFunctionRef(0), _stencilFunctionMask(RS_ALL_ONES)
        , _stencilOpSfail(RenderState::STENCIL_OP_KEEP), _stencilOpDpfail(RenderState::STENCIL_OP_KEEP), _stencilOpDppass(RenderState::STENCIL_OP_KEEP)
        , _bits(0L) {
    }

    RenderState::StateBlock::~StateBlock() {
    }

    void RenderState::StateBlock::bind() {
        // When the public bind() is called with no RenderState object passed in,
        // we assume we are being called to bind the state of a single StateBlock,
        // irrespective of whether it belongs to a hierarchy of RenderStates.
        // Therefore, we call restore() here with only this StateBlock's override
        // bits to restore state before applying the new state.
        StateBlock::restore(_bits);

        bindNoRestore();
    }

    void RenderState::StateBlock::bindNoRestore() {
        // Update any state that differs from _defaultState and flip _defaultState bits
        if ((_bits & RS_BLEND) && (_blendEnabled != _blendEnabled)) {
            if (_blendEnabled)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
            _blendEnabled = _blendEnabled;
        }
        if ((_bits & RS_BLEND_FUNC) && (_blendSrc != _blendSrc || _blendDst != _blendDst)) {
            GLStateCache::BlendFunc((GLenum)_blendSrc, (GLenum)_blendDst);
            _blendSrc = _blendSrc;
            _blendDst = _blendDst;
        }
        if ((_bits & RS_CULL_FACE) && (_cullFaceEnabled != _cullFaceEnabled)) {
            if (_cullFaceEnabled)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            _cullFaceEnabled = _cullFaceEnabled;
        }
        if ((_bits & RS_CULL_FACE_SIDE) && (_cullFaceSide != _cullFaceSide)) {
            glCullFace((GLenum)_cullFaceSide);
            _cullFaceSide = _cullFaceSide;
        }
        if ((_bits & RS_FRONT_FACE) && (_frontFace != _frontFace)) {
            glFrontFace((GLenum)_frontFace);
            _frontFace = _frontFace;
        }
        if ((_bits & RS_DEPTH_TEST) && (_depthTestEnabled != _depthTestEnabled)) {
            if (_depthTestEnabled)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
            _depthTestEnabled = _depthTestEnabled;
        }
        if ((_bits & RS_DEPTH_WRITE) && (_depthWriteEnabled != _depthWriteEnabled)) {
            glDepthMask(_depthWriteEnabled ? GL_TRUE : GL_FALSE);
            _depthWriteEnabled = _depthWriteEnabled;
        }
        if ((_bits & RS_DEPTH_FUNC) && (_depthFunction != _depthFunction)) {
            glDepthFunc((GLenum)_depthFunction);
            _depthFunction = _depthFunction;
        }

        _bits |= _bits;
    }

    void RenderState::StateBlock::restore(long stateOverrideBits) {
        // If there is no state to restore (i.e. no non-default state), do nothing.
        if ( (stateOverrideBits | _bits) == stateOverrideBits) {
            return;
        }

        // Restore any state that is not overridden and is not default
        if (!(stateOverrideBits & RS_BLEND) && (_bits & RS_BLEND)) {
            glEnable(GL_BLEND);
            _bits &= ~RS_BLEND;
            _blendEnabled = true;
        }
        if (!(stateOverrideBits & RS_BLEND_FUNC) && (_bits & RS_BLEND_FUNC)) {
            GLStateCache::BlendFunc(GL_ONE, GL_ZERO);
            _bits &= ~RS_BLEND_FUNC;
            _blendSrc = RenderState::BLEND_ONE;
            _blendDst = RenderState::BLEND_ZERO;
        }
        if (!(stateOverrideBits & RS_CULL_FACE) && (_bits & RS_CULL_FACE)) {
            glDisable(GL_CULL_FACE);
            _bits &= ~RS_CULL_FACE;
            _cullFaceEnabled = false;
        }
        if (!(stateOverrideBits & RS_CULL_FACE_SIDE) && (_bits & RS_CULL_FACE_SIDE)) {
            glCullFace((GLenum)GL_BACK);
            _bits &= ~RS_CULL_FACE_SIDE;
            _cullFaceSide = RenderState::CULL_FACE_SIDE_BACK;
        }
        if (!(stateOverrideBits & RS_FRONT_FACE) && (_bits & RS_FRONT_FACE)) {
            glFrontFace((GLenum)GL_CCW);
            _bits &= ~RS_FRONT_FACE;
            _frontFace = RenderState::FRONT_FACE_CCW;
        }
        if (!(stateOverrideBits & RS_DEPTH_TEST) && (_bits & RS_DEPTH_TEST)) {
            glEnable(GL_DEPTH_TEST);
            _bits &= ~RS_DEPTH_TEST;
            _depthTestEnabled = true;
        }
        if (!(stateOverrideBits & RS_DEPTH_WRITE) && (_bits & RS_DEPTH_WRITE)) {
            glDepthMask(GL_FALSE);
            _bits &= ~RS_DEPTH_WRITE;
            _depthWriteEnabled = false;
        }
        if (!(stateOverrideBits & RS_DEPTH_FUNC) && (_bits & RS_DEPTH_FUNC)) {
            glDepthFunc((GLenum)GL_LESS);
            _bits &= ~RS_DEPTH_FUNC;
            _depthFunction = RenderState::DEPTH_LESS;
        }
    }

    void RenderState::StateBlock::enableDepthWrite() {
        // Internal method used to restore depth writing before a
        // clear operation. This is necessary if the last code to draw before the
        // next frame leaves depth writing disabled.
        if (!_depthWriteEnabled) {
            glDepthMask(GL_TRUE);
            _bits &= ~RS_DEPTH_WRITE;
            _depthWriteEnabled = true;
        }
    }

    void RenderState::StateBlock::cloneInto(StateBlock* state) const {
        state->_cullFaceEnabled = _cullFaceEnabled;
        state->_depthTestEnabled = _depthTestEnabled;
        state->_depthWriteEnabled = _depthWriteEnabled;
        state->_depthFunction = _depthFunction;
        state->_blendEnabled = _blendEnabled;
        state->_blendSrc = _blendSrc;
        state->_blendDst = _blendDst;
        state->_cullFaceSide = _cullFaceSide;
        state->_frontFace = _frontFace;
        state->_stencilTestEnabled = _stencilTestEnabled;
        state->_stencilWrite = _stencilWrite;
        state->_stencilFunction = _stencilFunction;
        state->_stencilFunctionRef = _stencilFunctionRef;
        state->_stencilFunctionMask = _stencilFunctionMask;
        state->_stencilOpSfail = _stencilOpSfail;
        state->_stencilOpDpfail = _stencilOpDpfail;
        state->_stencilOpDppass = _stencilOpDppass;
        state->_bits = _bits;
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
        _bits = stateBits;
        restore(0);
    }

    void RenderState::StateBlock::setBlend(bool enabled) {
        _blendEnabled = enabled;
        if (enabled)
        {
            _bits &= ~RS_BLEND;
        }
        else
        {
            _bits |= RS_BLEND;
        }
    }

    void RenderState::StateBlock::setBlendFunc(const BlendFunc& blendFunc) {
        setBlendSrc((RenderState::Blend)blendFunc.src);
        setBlendDst((RenderState::Blend)blendFunc.dst);
    }

    void RenderState::StateBlock::setBlendSrc(Blend blend) {
        _blendSrc = blend;
        if (_blendSrc == BLEND_ONE && _blendDst == BLEND_ZERO) {
            // Default blend func
            _bits &= ~RS_BLEND_FUNC;
        }
        else {
            _bits |= RS_BLEND_FUNC;
        }
    }

    void RenderState::StateBlock::setBlendDst(Blend blend) {
        _blendDst = blend;
        if (_blendSrc == BLEND_ONE && _blendDst == BLEND_ZERO) {
            // Default blend func
            _bits &= ~RS_BLEND_FUNC;
        }
        else {
            _bits |= RS_BLEND_FUNC;
        }
    }

    void RenderState::StateBlock::setCullFace(bool enabled) {
        _cullFaceEnabled = enabled;
        if (!enabled) {
            _bits &= ~RS_CULL_FACE;
        }
        else {
            _bits |= RS_CULL_FACE;
        }
    }

    void RenderState::StateBlock::setCullFaceSide(CullFaceSide side) {
        _cullFaceSide = side;
        if (_cullFaceSide == CULL_FACE_SIDE_BACK) {
            // Default cull side
            _bits &= ~RS_CULL_FACE_SIDE;
        }
        else {
            _bits |= RS_CULL_FACE_SIDE;
        }
    }

    void RenderState::StateBlock::setFrontFace(FrontFace winding) {
        _frontFace = winding;
        if (_frontFace == FRONT_FACE_CCW) {
            // Default front face
            _bits &= ~RS_FRONT_FACE;
        }
        else {
            _bits |= RS_FRONT_FACE;
        }
    }

    void RenderState::StateBlock::setDepthTest(bool enabled) {
        _depthTestEnabled = enabled;
        if (enabled) {
            _bits &= ~RS_DEPTH_TEST;
        }
        else {
            _bits |= RS_DEPTH_TEST;
        }
    }

    void RenderState::StateBlock::setDepthWrite(bool enabled) {
        _depthWriteEnabled = enabled;
        if (!enabled) {
            _bits &= ~RS_DEPTH_WRITE;
        }
        else {
            _bits |= RS_DEPTH_WRITE;
        }
    }

    void RenderState::StateBlock::setDepthFunction(DepthFunction func) {
        _depthFunction = func;
        if (_depthFunction == DEPTH_LESS) {
            // Default depth function
            _bits &= ~RS_DEPTH_FUNC;
        }
        else {
            _bits |= RS_DEPTH_FUNC;
        }
    }
}
