#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <string.h>

namespace MATH
{
    class Vector4;
    class Matrix4x4;

    class Vector3
    {
    public:
        float x,y,z;

        Vector3() { }
        explicit Vector3(float f) {x=y=z=f;}

        float operator [] (int i) const { return (&x)[i]; }
        float &operator [] (int i) { return (&x)[i]; }

        Vector3(const float _x, const float _y, const float _z)
        {
            x=_x; y=_y; z=_z;
        }
        void Set(float _x, float _y, float _z)
        {
            x=_x; y=_y; z=_z;
        }
        Vector3 operator + (const Vector3 &other) const
        {
            return Vector3(x+other.x, y+other.y, z+other.z);
        }
        void operator += (const Vector3 &other)
        {
            x+=other.x; y+=other.y; z+=other.z;
        }
        Vector3 operator -(const Vector3 &v) const
        {
            return Vector3(x-v.x,y-v.y,z-v.z);
        }
        void operator -= (const Vector3 &other)
        {
            x-=other.x; y-=other.y; z-=other.z;
        }
        Vector3 operator -() const
        {
            return Vector3(-x,-y,-z);
        }

        Vector3 operator * (const float f) const
        {
            return Vector3(x*f,y*f,z*f);
        }
        Vector3 operator / (const float f) const
        {
            float invf = (1.0f/f);
            return Vector3(x*invf,y*invf,z*invf);
        }
        void operator /= (const float f)
        {
            *this = *this / f;
        }
        float operator * (const Vector3 &other) const
        {
            return x*other.x + y*other.y + z*other.z;
        }
        void operator *= (const float f)
        {
            *this = *this * f;
        }
        void scaleBy(const Vector3 &other)
        {
            x *= other.x; y *= other.y; z *= other.z;
        }
        Vector3 scaledBy(const Vector3 &other) const
        {
            return Vector3(x*other.x, y*other.y, z*other.z);
        }
        Vector3 scaledByInv(const Vector3 &other) const
        {
            return Vector3(x/other.x, y/other.y, z/other.z);
        }
        Vector3 operator *(const Matrix4x4 &m) const;
        void operator *=(const Matrix4x4 &m)
        {
            *this = *this * m;
        }
        Vector4 multiply4D(const Matrix4x4 &m) const;
        Vector3 rotatedBy(const Matrix4x4 &m) const;
        Vector3 operator %(const Vector3 &v) const
        {
            return Vector3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
        }
        float length2() const
        {
            return x*x + y*y + z*z;
        }
        float length() const
        {
            return sqrtf(length2());
        }
        void setLength(const float l)
        {
            (*this) *= l/length();
        }
        Vector3 withLength(const float l) const
        {
            return (*this) * l / length();
        }
        float distance2To(const Vector3 &other) const
        {
            return Vector3(other-(*this)).length2();
        }
        Vector3 normalized() const
        {
            return (*this) / length();
        }
        float normalize()
        { //returns the previous length, is often useful
            float len = length();
            (*this) = (*this)/len;
            return len;
        }
        bool operator == (const Vector3 &other) const
        {
            if (x==other.x && y==other.y && z==other.z)
                return true;
            else
                return false;
        }
        Vector3 lerp(const Vector3 &other, const float t) const
        {
            return (*this)*(1-t) + other*t;
        }
        void setZero()
        {
            memset((void *)this,0,sizeof(float)*3);
        }
    };

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

