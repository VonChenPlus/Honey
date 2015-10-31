#include "GRAPH/UNITY3D/Unity3DGLState.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
    int Unity3DGLState::state_count = 0;

    Unity3DGLState &Unity3DGLState::OpenGLState() {
        static Unity3DGLState instance;
        return instance;
    }

    void Unity3DGLState::restore() {
        int count = 0;

        useProgram.restore(); count++;

        blend.restore(); count++;
        blendEquationSeparate.restore(); count++;
        blendFuncSeparate.restore(); count++;
        blendColor.restore(); count++;

        scissorTest.restore(); count++;
        scissorRect.restore(); count++;

        cullFace.restore(); count++;
        cullFaceMode.restore(); count++;
        frontFace.restore(); count++;

        depthTest.restore(); count++;
        depthRange.restore(); count++;
        depthFunc.restore(); count++;
        depthWrite.restore(); count++;

        colorMask.restore(); count++;
        viewport.restore(); count++;

        stencilTest.restore(); count++;
        stencilOp.restore(); count++;
        stencilFunc.restore(); count++;
        stencilMask.restore(); count++;

        dither.restore(); count++;

        colorLogicOp.restore(); count++;
        logicOp.restore(); count++;

        arrayBuffer.restore(); count++;
        elementArrayBuffer.restore(); count++;

        if (count != state_count) {
            throw _HException_Normal("OpenGLState::Restore is missing some states");
        }
    }
}
