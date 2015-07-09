#ifndef VECTOR_H
#define VECTOR_H

#include <string.h>

#include "MATH/Vector2.h"
#include "MATH/Vector3.h"

namespace MATH
{
    class Vector4;
    class Matrix4x4;

    class Vector4
    {
    public:
        float x,y,z,w;
        Vector4(){}
        Vector4(float a, float b, float c, float d) {x=a;y=b;z=c;w=d;}
        Vector4 multiply4D(Matrix4x4 &m) const;
    };
}

#endif // VECTOR_H

