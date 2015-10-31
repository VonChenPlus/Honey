#include "Unity3D.h"
#include "GRAPH/UNITY3D/Unity3DGL.h"

namespace GRAPH
{
    Unity3DContext *Unity3DContext::CreateContext() {
        return new Unity3DGLContext;
    }

    Unity3DContext &Unity3DContext::DefaultContext() {
        static Unity3DGLContext instance;
        return instance;
    }
}
