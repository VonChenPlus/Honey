#include "MATH/Matrix.h"

#include "BASE/Honey.h"
#include "MATH/Quaternion.h"

#ifdef _WIN32
#undef far
#undef near
#endif

namespace MATH
{
    #define MATRIX_SIZE ( sizeof(float) * 16)

    Matrix4::Matrix4() {
        *this = IDENTITY;
    }

    Matrix4::Matrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
                   float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) {
        set(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
    }

    Matrix4::Matrix4(const float* mat) {
        set(mat);
    }

    Matrix4::Matrix4(const Matrix4& copy) {
        memcpy(m, copy.m, MATRIX_SIZE);
    }

    Matrix4::~Matrix4() {
    }

    void Matrix4::createLookAt(const Vector3f& eyePosition, const Vector3f& targetPosition, const Vector3f& up, Matrix4* dst) {
        createLookAt(eyePosition.x, eyePosition.y, eyePosition.z, targetPosition.x, targetPosition.y, targetPosition.z,
                     up.x, up.y, up.z, dst);
    }

    void Matrix4::createLookAt(float eyePositionX, float eyePositionY, float eyePositionZ,
                              float targetPositionX, float targetPositionY, float targetPositionZ,
                              float upX, float upY, float upZ, Matrix4* dst) {
        Vector3f eye(eyePositionX, eyePositionY, eyePositionZ);
        Vector3f target(targetPositionX, targetPositionY, targetPositionZ);
        Vector3f up(upX, upY, upZ);
        up.normalize();

        Vector3f zaxis;
        Vector3f::subtract(eye, target, &zaxis);
        zaxis.normalize();

        Vector3f xaxis;
        Vector3f::cross(up, zaxis, &xaxis);
        xaxis.normalize();

        Vector3f yaxis;
        Vector3f::cross(zaxis, xaxis, &yaxis);
        yaxis.normalize();

        dst->m[0] = xaxis.x;
        dst->m[1] = yaxis.x;
        dst->m[2] = zaxis.x;
        dst->m[3] = 0.0f;

        dst->m[4] = xaxis.y;
        dst->m[5] = yaxis.y;
        dst->m[6] = zaxis.y;
        dst->m[7] = 0.0f;

        dst->m[8] = xaxis.z;
        dst->m[9] = yaxis.z;
        dst->m[10] = zaxis.z;
        dst->m[11] = 0.0f;

        dst->m[12] = -Vector3f::dot(xaxis, eye);
        dst->m[13] = -Vector3f::dot(yaxis, eye);
        dst->m[14] = -Vector3f::dot(zaxis, eye);
        dst->m[15] = 1.0f;
    }

    void Matrix4::createPerspective(float fieldOfView, float aspectRatio,
                                         float zNearPlane, float zFarPlane, Matrix4* dst) {
        float f_n = 1.0f / (zFarPlane - zNearPlane);
        float theta = MATH_DEG_TO_RAD(fieldOfView) * 0.5f;
        if (fabs(fmod(theta, MATH_PIOVER2)) < MATH_FLOAT_EPSILON) {
            return;
        }
        float divisor = tan(theta);
        float factor = 1.0f / divisor;

        memset(dst, 0, MATRIX_SIZE);

        dst->m[0] = (1.0f / aspectRatio) * factor;
        dst->m[5] = factor;
        dst->m[10] = (-(zFarPlane + zNearPlane)) * f_n;
        dst->m[11] = -1.0f;
        dst->m[14] = -2.0f * zFarPlane * zNearPlane * f_n;
    }

    void Matrix4::createOrthographic(float width, float height, float zNearPlane, float zFarPlane, Matrix4* dst) {
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;
        createOrthographicOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNearPlane, zFarPlane, dst);
    }

    void Matrix4::createOrthographicOffCenter(float left, float right, float bottom, float top,
                                             float zNearPlane, float zFarPlane, Matrix4* dst) {
        memset(dst, 0, MATRIX_SIZE);
        dst->m[0] = 2 / (right - left);
        dst->m[5] = 2 / (top - bottom);
        dst->m[10] = 2 / (zNearPlane - zFarPlane);

        dst->m[12] = (left + right) / (left - right);
        dst->m[13] = (top + bottom) / (bottom - top);
        dst->m[14] = (zNearPlane + zFarPlane) / (zNearPlane - zFarPlane);
        dst->m[15] = 1;
    }

    void Matrix4::createBillboard(const Vector3f& objectPosition, const Vector3f& cameraPosition,
                                 const Vector3f& cameraUpVector, Matrix4* dst) {
        createBillboardHelper(objectPosition, cameraPosition, cameraUpVector, NULLPTR, dst);
    }

    void Matrix4::createBillboard(const Vector3f& objectPosition, const Vector3f& cameraPosition,
                                 const Vector3f& cameraUpVector, const Vector3f& cameraForwardVector,
                                 Matrix4* dst) {
        createBillboardHelper(objectPosition, cameraPosition, cameraUpVector, &cameraForwardVector, dst);
    }

    void Matrix4::createBillboardHelper(const Vector3f& objectPosition, const Vector3f& cameraPosition,
                                       const Vector3f& cameraUpVector, const Vector3f* cameraForwardVector,
                                       Matrix4* dst) {
        Vector3f delta(objectPosition, cameraPosition);
        bool isSufficientDelta = delta.lengthSquared() > MATH_FLOAT_EPSILON;

        dst->setIdentity();
        dst->m[3] = objectPosition.x;
        dst->m[7] = objectPosition.y;
        dst->m[11] = objectPosition.z;

        // As per the contracts for the 2 variants of createBillboard, we need
        // either a safe default or a sufficient distance between object and camera.
        if (cameraForwardVector || isSufficientDelta)
        {
            Vector3f target = isSufficientDelta ? cameraPosition : (objectPosition - *cameraForwardVector);

            // A billboard is the inverse of a lookAt rotation
            Matrix4 lookAt;
            createLookAt(objectPosition, target, cameraUpVector, &lookAt);
            dst->m[0] = lookAt.m[0];
            dst->m[1] = lookAt.m[4];
            dst->m[2] = lookAt.m[8];
            dst->m[4] = lookAt.m[1];
            dst->m[5] = lookAt.m[5];
            dst->m[6] = lookAt.m[9];
            dst->m[8] = lookAt.m[2];
            dst->m[9] = lookAt.m[6];
            dst->m[10] = lookAt.m[10];
        }
    }

    void Matrix4::createScale(const Vector3f& scale, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        dst->m[0] = scale.x;
        dst->m[5] = scale.y;
        dst->m[10] = scale.z;
    }

    void Matrix4::createScale(float xScale, float yScale, float zScale, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        dst->m[0] = xScale;
        dst->m[5] = yScale;
        dst->m[10] = zScale;
    }


    void Matrix4::createRotation(const Quaternion& q, Matrix4* dst) {
        float x2 = q.x + q.x;
        float y2 = q.y + q.y;
        float z2 = q.z + q.z;

        float xx2 = q.x * x2;
        float yy2 = q.y * y2;
        float zz2 = q.z * z2;
        float xy2 = q.x * y2;
        float xz2 = q.x * z2;
        float yz2 = q.y * z2;
        float wx2 = q.w * x2;
        float wy2 = q.w * y2;
        float wz2 = q.w * z2;

        dst->m[0] = 1.0f - yy2 - zz2;
        dst->m[1] = xy2 + wz2;
        dst->m[2] = xz2 - wy2;
        dst->m[3] = 0.0f;

        dst->m[4] = xy2 - wz2;
        dst->m[5] = 1.0f - xx2 - zz2;
        dst->m[6] = yz2 + wx2;
        dst->m[7] = 0.0f;

        dst->m[8] = xz2 + wy2;
        dst->m[9] = yz2 - wx2;
        dst->m[10] = 1.0f - xx2 - yy2;
        dst->m[11] = 0.0f;

        dst->m[12] = 0.0f;
        dst->m[13] = 0.0f;
        dst->m[14] = 0.0f;
        dst->m[15] = 1.0f;
    }

    void Matrix4::createRotation(const Vector3f& axis, float angle, Matrix4* dst) {
        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        // Make sure the input axis is normalized.
        float n = x*x + y*y + z*z;
        if (n != 1.0f)
        {
            // Not normalized.
            n = sqrt(n);
            // Prevent divide too close to zero.
            if (n > 0.000001f)
            {
                n = 1.0f / n;
                x *= n;
                y *= n;
                z *= n;
            }
        }

        float c = cos(angle);
        float s = sin(angle);

        float t = 1.0f - c;
        float tx = t * x;
        float ty = t * y;
        float tz = t * z;
        float txy = tx * y;
        float txz = tx * z;
        float tyz = ty * z;
        float sx = s * x;
        float sy = s * y;
        float sz = s * z;

        dst->m[0] = c + tx*x;
        dst->m[1] = txy + sz;
        dst->m[2] = txz - sy;
        dst->m[3] = 0.0f;

        dst->m[4] = txy - sz;
        dst->m[5] = c + ty*y;
        dst->m[6] = tyz + sx;
        dst->m[7] = 0.0f;

        dst->m[8] = txz + sy;
        dst->m[9] = tyz - sx;
        dst->m[10] = c + tz*z;
        dst->m[11] = 0.0f;

        dst->m[12] = 0.0f;
        dst->m[13] = 0.0f;
        dst->m[14] = 0.0f;
        dst->m[15] = 1.0f;
    }

    void Matrix4::createRotationX(float angle, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        float c = cos(angle);
        float s = sin(angle);

        dst->m[5]  = c;
        dst->m[6]  = s;
        dst->m[9]  = -s;
        dst->m[10] = c;
    }

    void Matrix4::createRotationY(float angle, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        float c = cos(angle);
        float s = sin(angle);

        dst->m[0]  = c;
        dst->m[2]  = -s;
        dst->m[8]  = s;
        dst->m[10] = c;
    }

    void Matrix4::createRotationZ(float angle, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        float c = cos(angle);
        float s = sin(angle);

        dst->m[0] = c;
        dst->m[1] = s;
        dst->m[4] = -s;
        dst->m[5] = c;
    }

    void Matrix4::createTranslation(const Vector3f& translation, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        dst->m[12] = translation.x;
        dst->m[13] = translation.y;
        dst->m[14] = translation.z;
    }

    void Matrix4::createTranslation(float xTranslation, float yTranslation, float zTranslation, Matrix4* dst) {
        memcpy(dst, &IDENTITY, MATRIX_SIZE);

        dst->m[12] = xTranslation;
        dst->m[13] = yTranslation;
        dst->m[14] = zTranslation;
    }

    void Matrix4::add(float scalar) {
        add(scalar, this);
    }

    void Matrix4::add(float scalar, Matrix4* matDst) {
        float *dst = *matDst;
        dst[0]  = m[0]  + scalar;
        dst[1]  = m[1]  + scalar;
        dst[2]  = m[2]  + scalar;
        dst[3]  = m[3]  + scalar;
        dst[4]  = m[4]  + scalar;
        dst[5]  = m[5]  + scalar;
        dst[6]  = m[6]  + scalar;
        dst[7]  = m[7]  + scalar;
        dst[8]  = m[8]  + scalar;
        dst[9]  = m[9]  + scalar;
        dst[10] = m[10] + scalar;
        dst[11] = m[11] + scalar;
        dst[12] = m[12] + scalar;
        dst[13] = m[13] + scalar;
        dst[14] = m[14] + scalar;
        dst[15] = m[15] + scalar;
    }

    void Matrix4::add(const Matrix4& mat) {
        add(*this, mat, this);
    }

    void Matrix4::add(const Matrix4& matSrc1, const Matrix4& matSrc2, Matrix4* matDst) {
        float *dst = *matDst;
        const float *m1 = matSrc1;
        const float *m2 = matSrc2;
        dst[0]  = m1[0]  + m2[0];
        dst[1]  = m1[1]  + m2[1];
        dst[2]  = m1[2]  + m2[2];
        dst[3]  = m1[3]  + m2[3];
        dst[4]  = m1[4]  + m2[4];
        dst[5]  = m1[5]  + m2[5];
        dst[6]  = m1[6]  + m2[6];
        dst[7]  = m1[7]  + m2[7];
        dst[8]  = m1[8]  + m2[8];
        dst[9]  = m1[9]  + m2[9];
        dst[10] = m1[10] + m2[10];
        dst[11] = m1[11] + m2[11];
        dst[12] = m1[12] + m2[12];
        dst[13] = m1[13] + m2[13];
        dst[14] = m1[14] + m2[14];
        dst[15] = m1[15] + m2[15];
    }

    bool Matrix4::decompose(Vector3f* scale, Quaternion* rotation, Vector3f* translation) const {
        if (translation) {
            // Extract the translation.
            translation->x = m[12];
            translation->y = m[13];
            translation->z = m[14];
        }

        // Nothing left to do.
        if (scale == NULLPTR && rotation == NULLPTR)
            return true;

        // Extract the scale.
        // This is simply the length of each axis (row/column) in the matrix.
        Vector3f xaxis(m[0], m[1], m[2]);
        float scaleX = xaxis.length();

        Vector3f yaxis(m[4], m[5], m[6]);
        float scaleY = yaxis.length();

        Vector3f zaxis(m[8], m[9], m[10]);
        float scaleZ = zaxis.length();

        // Determine if we have a negative scale (true if determinant is less than zero).
        // In this case, we simply negate a single axis of the scale.
        float det = determinant();
        if (det < 0)
            scaleZ = -scaleZ;

        if (scale)
        {
            scale->x = scaleX;
            scale->y = scaleY;
            scale->z = scaleZ;
        }

        // Nothing left to do.
        if (rotation == NULLPTR)
            return true;

        // Scale too close to zero, can't decompose rotation.
        if (scaleX < MATH_TOLERANCE || scaleY < MATH_TOLERANCE || fabs(scaleZ) < MATH_TOLERANCE)
            return false;

        float rn;

        // Factor the scale out of the matrix axes.
        rn = 1.0f / scaleX;
        xaxis.x *= rn;
        xaxis.y *= rn;
        xaxis.z *= rn;

        rn = 1.0f / scaleY;
        yaxis.x *= rn;
        yaxis.y *= rn;
        yaxis.z *= rn;

        rn = 1.0f / scaleZ;
        zaxis.x *= rn;
        zaxis.y *= rn;
        zaxis.z *= rn;

        // Now calculate the rotation from the resulting matrix (axes).
        float trace = xaxis.x + yaxis.y + zaxis.z + 1.0f;

        if (trace > MATH_FLOAT_EPSILON) {
            float s = 0.5f / sqrt(trace);
            rotation->w = 0.25f / s;
            rotation->x = (yaxis.z - zaxis.y) * s;
            rotation->y = (zaxis.x - xaxis.z) * s;
            rotation->z = (xaxis.y - yaxis.x) * s;
        }
        else {
            // Note: since xaxis, yaxis, and zaxis are normalized,
            // we will never divide by zero in the code below.
            if (xaxis.x > yaxis.y && xaxis.x > zaxis.z)
            {
                float s = 0.5f / sqrt(1.0f + xaxis.x - yaxis.y - zaxis.z);
                rotation->w = (yaxis.z - zaxis.y) * s;
                rotation->x = 0.25f / s;
                rotation->y = (yaxis.x + xaxis.y) * s;
                rotation->z = (zaxis.x + xaxis.z) * s;
            }
            else if (yaxis.y > zaxis.z)
            {
                float s = 0.5f / sqrt(1.0f + yaxis.y - xaxis.x - zaxis.z);
                rotation->w = (zaxis.x - xaxis.z) * s;
                rotation->x = (yaxis.x + xaxis.y) * s;
                rotation->y = 0.25f / s;
                rotation->z = (zaxis.y + yaxis.z) * s;
            }
            else
            {
                float s = 0.5f / sqrt(1.0f + zaxis.z - xaxis.x - yaxis.y );
                rotation->w = (xaxis.y - yaxis.x ) * s;
                rotation->x = (zaxis.x + xaxis.z ) * s;
                rotation->y = (zaxis.y + yaxis.z ) * s;
                rotation->z = 0.25f / s;
            }
        }

        return true;
    }

    float Matrix4::determinant() const {
        float a0 = m[0] * m[5] - m[1] * m[4];
        float a1 = m[0] * m[6] - m[2] * m[4];
        float a2 = m[0] * m[7] - m[3] * m[4];
        float a3 = m[1] * m[6] - m[2] * m[5];
        float a4 = m[1] * m[7] - m[3] * m[5];
        float a5 = m[2] * m[7] - m[3] * m[6];
        float b0 = m[8] * m[13] - m[9] * m[12];
        float b1 = m[8] * m[14] - m[10] * m[12];
        float b2 = m[8] * m[15] - m[11] * m[12];
        float b3 = m[9] * m[14] - m[10] * m[13];
        float b4 = m[9] * m[15] - m[11] * m[13];
        float b5 = m[10] * m[15] - m[11] * m[14];

        // Calculate the determinant.
        return (a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0);
    }

    void Matrix4::getScale(Vector3f* scale) const {
        decompose(scale, NULLPTR, NULLPTR);
    }

    bool Matrix4::getRotation(Quaternion* rotation) const {
        return decompose(NULLPTR, rotation, NULLPTR);
    }

    void Matrix4::getTranslation(Vector3f* translation) const {
        decompose(NULLPTR, NULLPTR, translation);
    }

    void Matrix4::getUpVector(Vector3f* dst) const {
        dst->x = m[4];
        dst->y = m[5];
        dst->z = m[6];
    }

    void Matrix4::getDownVector(Vector3f* dst) const {
        dst->x = -m[4];
        dst->y = -m[5];
        dst->z = -m[6];
    }

    void Matrix4::getLeftVector(Vector3f* dst) const {
        dst->x = -m[0];
        dst->y = -m[1];
        dst->z = -m[2];
    }

    void Matrix4::getRightVector(Vector3f* dst) const {
        dst->x = m[0];
        dst->y = m[1];
        dst->z = m[2];
    }

    void Matrix4::getForwardVector(Vector3f* dst) const {
        dst->x = -m[8];
        dst->y = -m[9];
        dst->z = -m[10];
    }

    void Matrix4::getBackVector(Vector3f* dst) const {
        dst->x = m[8];
        dst->y = m[9];
        dst->z = m[10];
    }

    Matrix4 Matrix4::getInversed() const {
        Matrix4 mat(*this);
        mat.inverse();
        return mat;
    }

    bool Matrix4::inverse() {
        float a0 = m[0] * m[5] - m[1] * m[4];
        float a1 = m[0] * m[6] - m[2] * m[4];
        float a2 = m[0] * m[7] - m[3] * m[4];
        float a3 = m[1] * m[6] - m[2] * m[5];
        float a4 = m[1] * m[7] - m[3] * m[5];
        float a5 = m[2] * m[7] - m[3] * m[6];
        float b0 = m[8] * m[13] - m[9] * m[12];
        float b1 = m[8] * m[14] - m[10] * m[12];
        float b2 = m[8] * m[15] - m[11] * m[12];
        float b3 = m[9] * m[14] - m[10] * m[13];
        float b4 = m[9] * m[15] - m[11] * m[13];
        float b5 = m[10] * m[15] - m[11] * m[14];

        // Calculate the determinant.
        float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

        // Close to zero, can't invert.
        if (fabs(det) <= MATH_TOLERANCE)
            return false;

        // Support the case where m == dst.
        Matrix4 inverse;
        inverse.m[0]  = m[5] * b5 - m[6] * b4 + m[7] * b3;
        inverse.m[1]  = -m[1] * b5 + m[2] * b4 - m[3] * b3;
        inverse.m[2]  = m[13] * a5 - m[14] * a4 + m[15] * a3;
        inverse.m[3]  = -m[9] * a5 + m[10] * a4 - m[11] * a3;

        inverse.m[4]  = -m[4] * b5 + m[6] * b2 - m[7] * b1;
        inverse.m[5]  = m[0] * b5 - m[2] * b2 + m[3] * b1;
        inverse.m[6]  = -m[12] * a5 + m[14] * a2 - m[15] * a1;
        inverse.m[7]  = m[8] * a5 - m[10] * a2 + m[11] * a1;

        inverse.m[8]  = m[4] * b4 - m[5] * b2 + m[7] * b0;
        inverse.m[9]  = -m[0] * b4 + m[1] * b2 - m[3] * b0;
        inverse.m[10] = m[12] * a4 - m[13] * a2 + m[15] * a0;
        inverse.m[11] = -m[8] * a4 + m[9] * a2 - m[11] * a0;

        inverse.m[12] = -m[4] * b3 + m[5] * b1 - m[6] * b0;
        inverse.m[13] = m[0] * b3 - m[1] * b1 + m[2] * b0;
        inverse.m[14] = -m[12] * a3 + m[13] * a1 - m[14] * a0;
        inverse.m[15] = m[8] * a3 - m[9] * a1 + m[10] * a0;

        multiply(inverse, 1.0f / det, this);

        return true;
    }

    bool Matrix4::isIdentity() const {
        return (memcmp(m, &IDENTITY, MATRIX_SIZE) == 0);
    }

    void Matrix4::multiply(float scalar) {
        multiply(scalar, this);
    }

    void Matrix4::multiply(float scalar, Matrix4* dst) const {
        multiply(*this, scalar, dst);
    }

    void Matrix4::multiply(const Matrix4& matSrc, float scalar, Matrix4* matDst) {
        float *dst = *matDst;
        const float *m = matSrc;
        dst[0]  = m[0]  * scalar;
        dst[1]  = m[1]  * scalar;
        dst[2]  = m[2]  * scalar;
        dst[3]  = m[3]  * scalar;
        dst[4]  = m[4]  * scalar;
        dst[5]  = m[5]  * scalar;
        dst[6]  = m[6]  * scalar;
        dst[7]  = m[7]  * scalar;
        dst[8]  = m[8]  * scalar;
        dst[9]  = m[9]  * scalar;
        dst[10] = m[10] * scalar;
        dst[11] = m[11] * scalar;
        dst[12] = m[12] * scalar;
        dst[13] = m[13] * scalar;
        dst[14] = m[14] * scalar;
        dst[15] = m[15] * scalar;
    }

    void Matrix4::multiply(const Matrix4& mat) {
        multiply(*this, mat, this);
    }

    void Matrix4::multiply(const Matrix4& matSrc1, const Matrix4& matSrc2, Matrix4* matDst) {
        float *dst = *matDst;
        const float *m1 = matSrc1;
        const float *m2 = matSrc2;
        float product[16];

        product[0]  = m1[0] * m2[0]  + m1[4] * m2[1] + m1[8]   * m2[2]  + m1[12] * m2[3];
        product[1]  = m1[1] * m2[0]  + m1[5] * m2[1] + m1[9]   * m2[2]  + m1[13] * m2[3];
        product[2]  = m1[2] * m2[0]  + m1[6] * m2[1] + m1[10]  * m2[2]  + m1[14] * m2[3];
        product[3]  = m1[3] * m2[0]  + m1[7] * m2[1] + m1[11]  * m2[2]  + m1[15] * m2[3];

        product[4]  = m1[0] * m2[4]  + m1[4] * m2[5] + m1[8]   * m2[6]  + m1[12] * m2[7];
        product[5]  = m1[1] * m2[4]  + m1[5] * m2[5] + m1[9]   * m2[6]  + m1[13] * m2[7];
        product[6]  = m1[2] * m2[4]  + m1[6] * m2[5] + m1[10]  * m2[6]  + m1[14] * m2[7];
        product[7]  = m1[3] * m2[4]  + m1[7] * m2[5] + m1[11]  * m2[6]  + m1[15] * m2[7];

        product[8]  = m1[0] * m2[8]  + m1[4] * m2[9] + m1[8]   * m2[10] + m1[12] * m2[11];
        product[9]  = m1[1] * m2[8]  + m1[5] * m2[9] + m1[9]   * m2[10] + m1[13] * m2[11];
        product[10] = m1[2] * m2[8]  + m1[6] * m2[9] + m1[10]  * m2[10] + m1[14] * m2[11];
        product[11] = m1[3] * m2[8]  + m1[7] * m2[9] + m1[11]  * m2[10] + m1[15] * m2[11];

        product[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8]  * m2[14] + m1[12] * m2[15];
        product[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9]  * m2[14] + m1[13] * m2[15];
        product[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
        product[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];

        memcpy(dst, product, MATRIX_SIZE);
    }

    void Matrix4::negate() {
        float *dst = *this;
        dst[0]  = -m[0];
        dst[1]  = -m[1];
        dst[2]  = -m[2];
        dst[3]  = -m[3];
        dst[4]  = -m[4];
        dst[5]  = -m[5];
        dst[6]  = -m[6];
        dst[7]  = -m[7];
        dst[8]  = -m[8];
        dst[9]  = -m[9];
        dst[10] = -m[10];
        dst[11] = -m[11];
        dst[12] = -m[12];
        dst[13] = -m[13];
        dst[14] = -m[14];
        dst[15] = -m[15];
    }

    Matrix4 Matrix4::getNegated() const {
        Matrix4 mat(*this);
        mat.negate();
        return mat;
    }

    void Matrix4::rotate(const Quaternion& q) {
        rotate(q, this);
    }

    void Matrix4::rotate(const Quaternion& q, Matrix4* dst) const {
        Matrix4 r;
        createRotation(q, &r);
        multiply(*this, r, dst);
    }

    void Matrix4::rotate(const Vector3f& axis, float angle) {
        rotate(axis, angle, this);
    }

    void Matrix4::rotate(const Vector3f& axis, float angle, Matrix4* dst) const {
        Matrix4 r;
        createRotation(axis, angle, &r);
        multiply(*this, r, dst);
    }

    void Matrix4::rotateX(float angle) {
        rotateX(angle, this);
    }

    void Matrix4::rotateX(float angle, Matrix4* dst) const {
        Matrix4 r;
        createRotationX(angle, &r);
        multiply(*this, r, dst);
    }

    void Matrix4::rotateY(float angle) {
        rotateY(angle, this);
    }

    void Matrix4::rotateY(float angle, Matrix4* dst) const {
        Matrix4 r;
        createRotationY(angle, &r);
        multiply(*this, r, dst);
    }

    void Matrix4::rotateZ(float angle) {
        rotateZ(angle, this);
    }

    void Matrix4::rotateZ(float angle, Matrix4* dst) const {
        Matrix4 r;
        createRotationZ(angle, &r);
        multiply(*this, r, dst);
    }

    void Matrix4::scale(float value) {
        scale(value, this);
    }

    void Matrix4::scale(float value, Matrix4* dst) const {
        scale(value, value, value, dst);
    }

    void Matrix4::scale(float xScale, float yScale, float zScale) {
        scale(xScale, yScale, zScale, this);
    }

    void Matrix4::scale(float xScale, float yScale, float zScale, Matrix4* dst) const {
        Matrix4 s;
        createScale(xScale, yScale, zScale, &s);
        multiply(*this, s, dst);
    }

    void Matrix4::scale(const Vector3f& s) {
        scale(s.x, s.y, s.z, this);
    }

    void Matrix4::scale(const Vector3f& s, Matrix4* dst) const {
        scale(s.x, s.y, s.z, dst);
    }

    void Matrix4::set(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
                     float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) {
        m[0]  = m11;
        m[1]  = m21;
        m[2]  = m31;
        m[3]  = m41;
        m[4]  = m12;
        m[5]  = m22;
        m[6]  = m32;
        m[7]  = m42;
        m[8]  = m13;
        m[9]  = m23;
        m[10] = m33;
        m[11] = m43;
        m[12] = m14;
        m[13] = m24;
        m[14] = m34;
        m[15] = m44;
    }

    void Matrix4::set(const float* mat) {
        memcpy(this->m, mat, MATRIX_SIZE);
    }

    void Matrix4::set(const Matrix4& mat) {
        memcpy(this->m, mat.m, MATRIX_SIZE);
    }

    void Matrix4::setIdentity() {
        memcpy(m, &IDENTITY, MATRIX_SIZE);
    }

    void Matrix4::setZero() {
        memset(m, 0, MATRIX_SIZE);
    }

    void Matrix4::subtract(const Matrix4& mat) {
        subtract(*this, mat, this);
    }

    void Matrix4::subtract(const Matrix4& matSrc1, const Matrix4& matSrc2, Matrix4* matDst) {
        float *dst = *matDst;
        const float *m1 = matSrc1;
        const float *m2 = matSrc2;
        dst[0]  = m1[0]  - m2[0];
        dst[1]  = m1[1]  - m2[1];
        dst[2]  = m1[2]  - m2[2];
        dst[3]  = m1[3]  - m2[3];
        dst[4]  = m1[4]  - m2[4];
        dst[5]  = m1[5]  - m2[5];
        dst[6]  = m1[6]  - m2[6];
        dst[7]  = m1[7]  - m2[7];
        dst[8]  = m1[8]  - m2[8];
        dst[9]  = m1[9]  - m2[9];
        dst[10] = m1[10] - m2[10];
        dst[11] = m1[11] - m2[11];
        dst[12] = m1[12] - m2[12];
        dst[13] = m1[13] - m2[13];
        dst[14] = m1[14] - m2[14];
        dst[15] = m1[15] - m2[15];
    }

    void Matrix4::transformVector(Vector3f* vector) const {
        transformVector(vector->x, vector->y, vector->z, 0.0f, vector);
    }

    void Matrix4::transformVector(const Vector3f& vector, Vector3f* dst) const {
        transformVector(vector.x, vector.y, vector.z, 0.0f, dst);
    }

    void Matrix4::transformVector(float x, float y, float z, float w, Vector3f* vec3Dst) const {
        float *dst = *vec3Dst;
        dst[0] = x * m[0] + y * m[4] + z * m[8] + w * m[12];
        dst[1] = x * m[1] + y * m[5] + z * m[9] + w * m[13];
        dst[2] = x * m[2] + y * m[6] + z * m[10] + w * m[14];
    }

    void Matrix4::transformVector(Vector4f* vector) const {
        transformVector(*vector, vector);
    }

    void Matrix4::transformVector(const Vector4f& vec3Src, Vector4f* vec3Dst) const {
        const float *v = vec3Src;
        float *dst = *vec3Dst;

        float x = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
        float y = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
        float z = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
        float w = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];

        dst[0] = x;
        dst[1] = y;
        dst[2] = z;
        dst[3] = w;
    }

    void Matrix4::translate(float x, float y, float z) {
        translate(x, y, z, this);
    }

    void Matrix4::translate(float x, float y, float z, Matrix4* dst) const {
        Matrix4 t;
        createTranslation(x, y, z, &t);
        multiply(*this, t, dst);
    }

    void Matrix4::translate(const Vector3f& t) {
        translate(t.x, t.y, t.z, this);
    }

    void Matrix4::translate(const Vector3f& t, Matrix4* dst) const {
        translate(t.x, t.y, t.z, dst);
    }

    void Matrix4::transpose() {
        float t[16] = {
            m[0], m[4], m[8], m[12],
            m[1], m[5], m[9], m[13],
            m[2], m[6], m[10], m[14],
            m[3], m[7], m[11], m[15]
        };
        float *dst = *this;
        memcpy(dst, t, MATRIX_SIZE);
    }

    Matrix4 Matrix4::getTransposed() const {
        Matrix4 mat(*this);
        mat.transpose();
        return mat;
    }

    inline const Matrix4 Matrix4::operator+(const Matrix4& mat) const {
        Matrix4 result(*this);
        result.add(mat);
        return result;
    }

    inline Matrix4& Matrix4::operator+=(const Matrix4& mat) {
        add(mat);
        return *this;
    }

    inline const Matrix4 Matrix4::operator-(const Matrix4& mat) const {
        Matrix4 result(*this);
        result.subtract(mat);
        return result;
    }

    inline Matrix4& Matrix4::operator-=(const Matrix4& mat) {
        subtract(mat);
        return *this;
    }

    inline const Matrix4 Matrix4::operator-() const {
        Matrix4 mat(*this);
        mat.negate();
        return mat;
    }

    inline const Matrix4 Matrix4::operator*(const Matrix4& mat) const {
        Matrix4 result(*this);
        result.multiply(mat);
        return result;
    }

    inline Matrix4& Matrix4::operator*=(const Matrix4& mat) {
        multiply(mat);
        return *this;
    }

    const Matrix4 Matrix4::IDENTITY = Matrix4(
                        1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);

    const Matrix4 Matrix4::ZERO = Matrix4(
                        0, 0, 0, 0,
                        0, 0, 0, 0,
                        0, 0, 0, 0,
                        0, 0, 0, 0 );
}
