#ifndef TYPES_H
#define TYPES_H

#include "MATH/Vector.h"
#include "MATH/Size.h"
#include "GRAPH/Color.h"
#include "GRAPH/UNITY3D/GLCommon.h"

namespace GRAPH
{
    struct BlendFunc
    {
        GLenum src;
        GLenum dst;

        static const BlendFunc DISABLE;
        static const BlendFunc ALPHA_PREMULTIPLIED;
        static const BlendFunc ALPHA_NON_PREMULTIPLIED;
        static const BlendFunc ADDITIVE;

        bool operator==(const BlendFunc &a) const {
            return src == a.src && dst == a.dst;
        }

        bool operator!=(const BlendFunc &a) const {
            return src != a.src || dst != a.dst;
        }

        bool operator<(const BlendFunc &a) const {
            return src < a.src || (src == a.src && dst < a.dst);
        }
    };
}

#endif // TYPES_H
