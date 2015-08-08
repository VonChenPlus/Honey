#ifndef MATRIX_H
#define MATRIX_H

#include "MATH/Vector.h"

namespace MATH
{
    class Quaternion;

    class Matrix4 final
    {
    public:
        Matrix4();

        Matrix4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
               float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);

        Matrix4(const float* mat);

        Matrix4(const Matrix4& copy);

        ~Matrix4();

        static void createLookAt(const Vector3f& eyePosition, const Vector3f& targetPosition, const Vector3f& up, Matrix4* dst);
        static void createLookAt(float eyePositionX, float eyePositionY, float eyePositionZ,
                                 float targetCenterX, float targetCenterY, float targetCenterZ,
                                 float upX, float upY, float upZ, Matrix4* dst);
        static void createPerspective(float fieldOfView, float aspectRatio, float zNearPlane, float zFarPlane, Matrix4* dst);
        static void createOrthographic(float width, float height, float zNearPlane, float zFarPlane, Matrix4* dst);
        static void createOrthographicOffCenter(float left, float right, float bottom, float top,
                                                float zNearPlane, float zFarPlane, Matrix4* dst);
        static void createBillboard(const Vector3f& objectPosition, const Vector3f& cameraPosition,
                                    const Vector3f& cameraUpVector, Matrix4* dst);
        static void createBillboard(const Vector3f& objectPosition, const Vector3f& cameraPosition,
                                    const Vector3f& cameraUpVector, const Vector3f& cameraForwardVector,
                                    Matrix4* dst);
        static void createScale(const Vector3f& scale, Matrix4* dst);
        static void createScale(float xScale, float yScale, float zScale, Matrix4* dst);
        static void createRotation(const Quaternion& quat, Matrix4* dst);
        static void createRotation(const Vector3f& axis, float angle, Matrix4* dst);
        static void createRotationX(float angle, Matrix4* dst);
        static void createRotationY(float angle, Matrix4* dst);
        static void createRotationZ(float angle, Matrix4* dst);
        static void createTranslation(const Vector3f& translation, Matrix4* dst);
        static void createTranslation(float xTranslation, float yTranslation, float zTranslation, Matrix4* dst);
        static void add(const Matrix4& m1, const Matrix4& m2, Matrix4* dst);
        static void multiply(const Matrix4& mat, float scalar, Matrix4* dst);
        static void multiply(const Matrix4& m1, const Matrix4& m2, Matrix4* dst);
        static void subtract(const Matrix4& m1, const Matrix4& m2, Matrix4* dst);

        void add(float scalar);
        void add(float scalar, Matrix4* dst);
        void add(const Matrix4& mat);

        bool decompose(Vector3f* scale, Quaternion* rotation, Vector3f* translation) const;

        float determinant() const;

        void getScale(Vector3f* scale) const;
        bool getRotation(Quaternion* rotation) const;
        void getTranslation(Vector3f* translation) const;
        void getUpVector(Vector3f* dst) const;
        void getDownVector(Vector3f* dst) const;
        void getLeftVector(Vector3f* dst) const;
        void getRightVector(Vector3f* dst) const;
        void getForwardVector(Vector3f* dst) const;
        void getBackVector(Vector3f* dst) const;

        bool inverse();

        Matrix4 getInversed() const;

        bool isIdentity() const;

        void multiply(float scalar);
        void multiply(float scalar, Matrix4* dst) const;
        void multiply(const Matrix4& mat);

        void negate();
        Matrix4 getNegated() const;

        void rotate(const Quaternion& q);
        void rotate(const Quaternion& q, Matrix4* dst) const;
        void rotate(const Vector3f& axis, float angle);
        void rotate(const Vector3f& axis, float angle, Matrix4* dst) const;
        void rotateX(float angle);
        void rotateX(float angle, Matrix4* dst) const;
        void rotateY(float angle);
        void rotateY(float angle, Matrix4* dst) const;
        void rotateZ(float angle);
        void rotateZ(float angle, Matrix4* dst) const;

        void scale(float value);
        void scale(float value, Matrix4* dst) const;
        void scale(float xScale, float yScale, float zScale);
        void scale(float xScale, float yScale, float zScale, Matrix4* dst) const;
        void scale(const Vector3f& s);
        void scale(const Vector3f& s, Matrix4* dst) const;

        void set(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
                 float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);
        void set(const float* mat);
        void set(const Matrix4& mat);
        void setIdentity();
        void setZero();

        void subtract(const Matrix4& mat);

        inline void transformPoint(Vector3f* point) const { transformVector(point->x, point->y, point->z, 1.0f, point); }
        inline void transformPoint(const Vector3f& point, Vector3f* dst) const { transformVector(point.x, point.y, point.z, 1.0f, dst); }
        void transformVector(Vector3f* vector) const;
        void transformVector(const Vector3f& vector, Vector3f* dst) const;
        void transformVector(float x, float y, float z, float w, Vector3f* dst) const;
        void transformVector(Vector4f* vector) const;
        void transformVector(const Vector4f& vector, Vector4f* dst) const;
        void translate(float x, float y, float z);
        void translate(float x, float y, float z, Matrix4* dst) const;
        void translate(const Vector3f& t);
        void translate(const Vector3f& t, Matrix4* dst) const;
        void transpose();

        Matrix4 getTransposed() const;

        inline Matrix4 operator+(const Matrix4& mat) const {
            Matrix4 result(*this);
            result.add(mat);
            return result;
        }

        inline Matrix4& operator+=(const Matrix4& mat) {
            add(mat);
            return *this;
        }

        inline Matrix4 operator-(const Matrix4& mat) const {
            Matrix4 result(*this);
            result.subtract(mat);
            return result;
        }

        inline Matrix4& operator-=(const Matrix4& mat) {
            subtract(mat);
            return *this;
        }

        inline Matrix4 operator-() const {
            Matrix4 mat(*this);
            mat.negate();
            return mat;
        }

        inline Matrix4 operator*(const Matrix4& mat) const {
            Matrix4 result(*this);
            result.multiply(mat);
            return result;
        }

        inline Matrix4& operator*=(const Matrix4& mat) {
            multiply(mat);
            return *this;
        }

        inline operator float *() {
            return (float *)this;
        }

        inline operator const float *() const {
            return (const float *)this;
        }

        inline const float &operator[](int i) const {
            return *(((const float *)this) + i);
        }
        inline float &operator[](int i) {
            return *(((float *)this) + i);
        }

        /** equals to a matrix full of zeros */
        static const Matrix4 ZERO;
        /** equals to the identity matrix */
        static const Matrix4 IDENTITY;

    private:
        static void createBillboardHelper(const Vector3f& objectPosition, const Vector3f& cameraPosition,
                                          const Vector3f& cameraUpVector, const Vector3f* cameraForwardVector,
                                          Matrix4* dst);

    public:
        float m[16];
    };
}

#endif // MATRIX_H

