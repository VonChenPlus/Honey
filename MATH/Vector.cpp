#include "MATH/Vector.h"
#include "MATH/Matrix.h"

namespace MATH
{
    Vector3 Vector3::operator *(const Matrix4x4 &m) const
    {
        return Vector3(x*m.xx + y*m.yx + z*m.zx + m.wx,
            x*m.xy + y*m.yy + z*m.zy + m.wy,
            x*m.xz + y*m.yz + z*m.zz + m.wz);
    }

    Vector4 Vector3::multiply4D(const Matrix4x4 &m) const
    {
        return Vector4(x*m.xx + y*m.yx + z*m.zx + m.wx,
            x*m.xy + y*m.yy + z*m.zy + m.wy,
            x*m.xz + y*m.yz + z*m.zz + m.wz,
            x*m.xw + y*m.yw + z*m.zw + m.ww);
    }

    Vector4 Vector4::multiply4D(Matrix4x4 &m) const
    {
        return Vector4(x*m.xx + y*m.yx + z*m.zx + w*m.wx,
            x*m.xy + y*m.yy + z*m.zy + w*m.wy,
            x*m.xz + y*m.yz + z*m.zz + w*m.wz,
            x*m.xw + y*m.yw + z*m.zw + w*m.ww);
    }

    Vector3 Vector3::rotatedBy(const Matrix4x4 &m) const
    {
        return Vector3(x*m.xx + y*m.yx + z*m.zx,
            x*m.xy + y*m.yy + z*m.zy,
            x*m.xz + y*m.yz + z*m.zz);
    }
}
