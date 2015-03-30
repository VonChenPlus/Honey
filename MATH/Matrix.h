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

        const Vector3 right() const {return Vector3(xx, xy, xz);}
        const Vector3 up()		const {return Vector3(yx, yy, yz);}
        const Vector3 front() const {return Vector3(zx, zy, zz);}
        const Vector3 move()	const {return Vector3(wx, wy, wz);}

        void setRight(const Vector3 &v)
        {
            xx = v.x; xy = v.y; xz = v.z;
        }
        void setUp(const Vector3 &v)
        {
            yx = v.x; yy = v.y; yz = v.z;
        }
        void setFront(const Vector3 &v)
        {
            zx = v.x; zy = v.y; zz = v.z;
        }
        void setMove(const Vector3 &v)
        {
            wx = v.x; wy = v.y; wz = v.z;
        }


        const float &operator[](int i) const
        {
            return *(((const float *)this) + i);
        }
        float &operator[](int i)
        {
            return *(((float *)this) + i);
        }
        Matrix4x4 operator * (const Matrix4x4 &other) const ;
        void operator *= (const Matrix4x4 &other)
        {
            *this = *this * other;
        }
        const float *getReadPtr() const
        {
            return (const float *)this;
        }
        void empty()
        {
            memset(this, 0, 16 * sizeof(float));
        }
        void setScaling(const float f)
        {
            empty();
            xx=yy=zz=f; ww=1.0f;
        }
        void setScaling(const Vector3 f)
        {
            empty();
            xx=f.x;
            yy=f.y;
            zz=f.z;
            ww=1.0f;
        }

        void setIdentity()
        {
            setScaling(1.0f);
        }
        void setTranslation(const Vector3 &trans)
        {
            setIdentity();
            wx = trans.x;
            wy = trans.y;
            wz = trans.z;
        }
        void setTranslationAndScaling(const Vector3 &trans, const Vector3 &scale)
        {
            setScaling(scale);
            wx = trans.x;
            wy = trans.y;
            wz = trans.z;
        }

        Matrix4x4 inverse() const;
        Matrix4x4 simpleInverse() const;
        Matrix4x4 transpose() const;

        void setRotationX(const float a)
        {
            empty();
            float c=cosf(a);
            float s=sinf(a);
            xx = 1.0f;
            yy =	c;			yz = s;
            zy = -s;			zz = c;
            ww = 1.0f;
        }
        void setRotationY(const float a)
        {
            empty();
            float c=cosf(a);
            float s=sinf(a);
            xx = c;									 xz = -s;
            yy =	1.0f;
            zx = s;									 zz = c	;
            ww = 1.0f;
        }
        void setRotationZ(const float a)
        {
            empty();
            float c=cosf(a);
            float s=sinf(a);
            xx = c;		xy = s;
            yx = -s;	 yy = c;
            zz = 1.0f;
            ww = 1.0f;
        }

        void setRotationAxisAngle(const Vector3 &axis, float angle);
        void setRotation(float x,float y, float z);
        void setProjection(float near_plane, float far_plane, float fov_horiz, float aspect = 0.75f);
        void setProjectionD3D(float near_plane, float far_plane, float fov_horiz, float aspect = 0.75f);
        void setProjectionInf(float near_plane, float fov_horiz, float aspect = 0.75f);
        void setOrtho(float left, float right, float bottom, float top, float near, float far);
        void setOrthoD3D(float left, float right, float bottom, float top, float near, float far);
        void setShadow(float Lx, float Ly, float Lz, float Lw)
        {
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

        void setViewLookAt(const Vector3 &from, const Vector3 &at, const Vector3 &worldup);
        void setViewLookAtD3D(const Vector3 &from, const Vector3 &at, const Vector3 &worldup);
        void setViewFrame(const Vector3 &pos, const Vector3 &right, const Vector3 &forward, const Vector3 &up);
        void stabilizeOrtho()
        {
            /*
            front().normalize();
            right().normalize();
            up() = front() % right();
            right() = up() % front();
            */
        }
        void toText(char *buffer, int len) const;
        void print() const;
        static Matrix4x4 fromPRS(const Vector3 &position, const Quaternion &normal, const Vector3 &scale);

        void translateAndScale(const Vector3 &trans, const Vector3 &scale)
        {
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
}

#endif // MATRIX_H

