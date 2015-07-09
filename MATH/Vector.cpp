#include "MATH/Vector.h"
#include "MATH/Matrix.h"

namespace MATH
{
    Vector4 Vector4::multiply4D(Matrix4x4 &m) const {
        return Vector4(x*m.xx + y*m.yx + z*m.zx + w*m.wx,
            x*m.xy + y*m.yy + z*m.zy + w*m.wy,
            x*m.xz + y*m.yz + z*m.zz + w*m.wz,
            x*m.xw + y*m.yw + z*m.zw + w*m.ww);
    }
}
