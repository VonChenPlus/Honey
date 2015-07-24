#include "MATH/Quaternion.h"
#include "MATH/Matrix.h"
#include "BASE/Honey.h"

namespace MATH
{
    // TODO: Allegedly, lerp + normalize can achieve almost as good results.
    Quaternion Quaternion::slerp(const Quaternion &to, const float a) const {
        Quaternion to2;
        float angle, cos_angle, scale_from, scale_to, sin_angle;

        cos_angle = (x * to.x) + (y * to.y) + (z * to.z) + (w * to.w);	//4D dot product

        if (cos_angle < 0.0f) {
            cos_angle = -cos_angle;
            to2.w = -to.w; to2.x = -to.x; to2.y = -to.y; to2.z = -to.z;
        }
        else {
            to2 = to;
        }

        if ((1.0f - fabsf(cos_angle)) > 0.00001f) {
            /* spherical linear interpolation (SLERP) */
            angle = acosf(cos_angle);
            sin_angle	= sinf(angle);
            scale_from = sinf((1.0f - a) * angle) / sin_angle;
            scale_to	 = sinf(a				 * angle) / sin_angle;
        }
        else {
            /* to prevent divide-by-zero, resort to linear interpolation */
            // This is okay in 99% of cases anyway, maybe should be the default?
            scale_from = 1.0f - a;
            scale_to	 = a;
        }

        return Quaternion(
            scale_from*x + scale_to*to2.x,
            scale_from*y + scale_to*to2.y,
            scale_from*z + scale_to*to2.z,
            scale_from*w + scale_to*to2.w
            );
    }

    Quaternion Quaternion::multiply(const Quaternion &q) const {
        return Quaternion((w * q.x) + (x * q.w) + (y * q.z) - (z * q.y),
            (w * q.y) + (y * q.w) + (z * q.x) - (x * q.z),
            (w * q.z) + (z * q.w) + (x * q.y) - (y * q.x),
            (w * q.w) - (x * q.x) - (y * q.y) - (z * q.z));
    }
}
