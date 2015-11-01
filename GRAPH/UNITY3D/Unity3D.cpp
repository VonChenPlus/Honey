#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/UNITY3D/Unity3DGL.h"

namespace GRAPH
{
    Unity3DCreator::RenderEngine Unity3DCreator::EngineMode = OPENGL;

    Unity3DContext *Unity3DCreator::CreateContext() {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateContext();
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DDepthState *Unity3DCreator::CreateDepthState() {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateDepthState();
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DBuffer *Unity3DCreator::CreateBuffer(uint32 usageFlags) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateBuffer(usageFlags);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DShaderSet *Unity3DCreator::CreateShaderSet(Unity3DShader *vshader, Unity3DShader *fshader) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateShaderSet(vshader, fshader);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DVertexFormat *Unity3DCreator::CreateVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateVertexFormat(components, stride);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }
}
