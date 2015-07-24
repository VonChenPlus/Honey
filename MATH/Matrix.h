#ifndef MATRIX_H
#define MATRIX_H

#include "MATH/Vector.h"

namespace MATH
{
    class Quaternion;

    class Matrix4x4
    {
    public:
        union
        {
            struct
            {
                float xx, xy, xz, xw;
                float yx, yy, yz, yw;
                float zx, zy, zz, zw;
                float wx, wy, wz, ww;
            };
            float m[16];
        };

        const Vector3f right() const {return Vector3f(xx, xy, xz);}
        const Vector3f up()		const {return Vector3f(yx, yy, yz);}
        const Vector3f front() const {return Vector3f(zx, zy, zz);}
        const Vector3f move()	const {return Vector3f(wx, wy, wz);}

        void setRight(const Vector3f &v) {
            xx = v.x; xy = v.y; xz = v.z;
        }
        void setUp(const Vector3f &v) {
            yx = v.x; yy = v.y; yz = v.z;
        }
        void setFront(const Vector3f &v) {
            zx = v.x; zy = v.y; zz = v.z;
        }
        void setMove(const Vector3f &v) {
            wx = v.x; wy = v.y; wz = v.z;
        }


        const float &operator[](int i) const {
            return *(((const float *)this) + i);
        }
        float &operator[](int i) {
            return *(((float *)this) + i);
        }
        Matrix4x4 operator * (const Matrix4x4 &other) const ;
        void operator *= (const Matrix4x4 &other) {
            *this = *this * other;
        }
        const float *getReadPtr() const {
            return (const float *)this;
        }
        void empty() {
            memset(this, 0, 16 * sizeof(float));
        }
        void setScaling(const float f) {
            empty();
            xx=yy=zz=f; ww=1.0f;
        }
        void setScaling(const Vector3f f) {
            empty();
            xx=f.x;
            yy=f.y;
            zz=f.z;
            ww=1.0f;
        }

        void setIdentity() {
            setScaling(1.0f);
        }
        void setTranslation(const Vector3f &trans) {
            setIdentity();
            wx = trans.x;
            wy = trans.y;
            wz = trans.z;
        }
        void setTranslationAndScaling(const Vector3f &trans, const Vector3f &scale) {
            setScaling(scale);
            wx = trans.x;
            wy = trans.y;
            wz = trans.z;
        }

        Matrix4x4 inverse() const;
        Matrix4x4 simpleInverse() const;
        Matrix4x4 transpose() const;

        void setRotationX(const float a) {
            empty();
            float c=cosf(a);
            float s=sinf(a);
            xx = 1.0f;
            yy =	c;			yz = s;
            zy = -s;			zz = c;
            ww = 1.0f;
        }
        void setRotationY(const float a) {
            empty();
            float c=cosf(a);
            float s=sinf(a);
            xx = c;									 xz = -s;
            yy =	1.0f;
            zx = s;									 zz = c	;
            ww = 1.0f;
        }
        void setRotationZ(const float a) {
            empty();
            float c=cosf(a);
            float s=sinf(a);
            xx = c;		xy = s;
            yx = -s;	 yy = c;
            zz = 1.0f;
            ww = 1.0f;
        }

        void setRotationAxisAngle(const Vector3f &axis, float angle);
        void setRotation(float x,float y, float z);
        void setProjection(float near_plane, float far_plane, float fov_horiz, float aspect = 0.75f);
        void setProjectionD3D(float near_plane, float far_plane, float fov_horiz, float aspect = 0.75f);
        void setProjectionInf(float near_plane, float fov_horiz, float aspect = 0.75f);
        void setOrtho(float left, float right, float bottom, float top, float near, float far);
        void setOrthoD3D(float left, float right, float bottom, float top, float near, float far);
        void setShadow(float Lx, float Ly, float Lz, float Lw) {
            float Pa=0;
            float Pb=1;
            float Pc=0;
            float Pd=0;
            //P = normalize(Plane);
            float d = (Pa*Lx + Pb*Ly + Pc*Lz + Pd*Lw);

            xx=Pa * Lx + d;	xy=Pa * Ly;		 xz=Pa * Lz;		 xw=Pa * Lw;
            yx=Pb * Lx;			yy=Pb * Ly + d; yz=Pb * Lz;		 yw=Pb * Lw;
            zx=Pc * Lx;			zy=Pc * Ly;		 zz=Pc * Lz + d; zw=Pc * Lw;
            wx=Pd * Lx;			wy=Pd * Ly;		 wz=Pd * Lz;		 ww=Pd * Lw + d;
        }

        void setViewLookAt(const Vector3f &from, const Vector3f &at, const Vector3f &worldup);
        void setViewLookAtD3D(const Vector3f &from, const Vector3f &at, const Vector3f &worldup);
        void setViewFrame(const Vector3f &pos, const Vector3f &right, const Vector3f &forward, const Vector3f &up);
        void stabilizeOrtho() {
            /*
            front().normalize();
            right().normalize();
            up() = front() % right();
            right() = up() % front();
            */
        }
        void toText(char *buffer, int len) const;
        void print() const;
        static Matrix4x4 fromPRS(const Vector3f &position, const Quaternion &normal, const Vector3f &scale);

        void translateAndScale(const Vector3f &trans, const Vector3f &scale) {
            xx = xx * scale.x + xw * trans.x;
            xy = xy * scale.y + xw * trans.y;
            xz = xz * scale.z + xw * trans.z;

            yx = yx * scale.x + yw * trans.x;
            yy = yy * scale.y + yw * trans.y;
            yz = yz * scale.z + yw * trans.z;

            zx = zx * scale.x + zw * trans.x;
            zy = zy * scale.y + zw * trans.y;
            zz = zz * scale.z + zw * trans.z;

            wx = wx * scale.x + ww * trans.x;
            wy = wy * scale.y + ww * trans.y;
            wz = wz * scale.z + ww * trans.z;
        }
    };

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

        void add(float scalar);

        void add(float scalar, Matrix4* dst);

        void add(const Matrix4& mat);

        static void add(const Matrix4& m1, const Matrix4& m2, Matrix4* dst);

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
        static void multiply(const Matrix4& mat, float scalar, Matrix4* dst);

        void multiply(const Matrix4& mat);
        static void multiply(const Matrix4& m1, const Matrix4& m2, Matrix4* dst);

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
        static void subtract(const Matrix4& m1, const Matrix4& m2, Matrix4* dst);

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

        inline const Matrix4 operator+(const Matrix4& mat) const;
        inline Matrix4& operator+=(const Matrix4& mat);
        inline const Matrix4 operator-(const Matrix4& mat) const;
        inline Matrix4& operator-=(const Matrix4& mat);
        inline const Matrix4 operator-() const;
        inline const Matrix4 operator*(const Matrix4& mat) const;
        inline Matrix4& operator*=(const Matrix4& mat);

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

