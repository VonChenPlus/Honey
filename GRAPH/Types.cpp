#include "Types.h"
#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    const BlendFunc BlendFunc::DISABLE = { ONE, ZERO };
    const BlendFunc BlendFunc::ALPHA_PREMULTIPLIED = { ONE, ONE_MINUS_SRC_ALPHA };
    const BlendFunc BlendFunc::ALPHA_NON_PREMULTIPLIED = { SRC_ALPHA, ONE_MINUS_SRC_ALPHA };
    const BlendFunc BlendFunc::ADDITIVE = { SRC_ALPHA, ONE };
}
