#ifndef SIZE_H
#define SIZE_H

#include "MATH/Vector2.h"

namespace MATH
{
    template <typename T>
    class Size
    {
    public:
        Size()
            : width(MATHZERO<T>())
            , height(MATHZERO<T>()) {

        }

        Size(T _width, T _height)
            : width(_width)
            , height(_height) {

        }

        Size(const Size& other)
            : width(other.width)
            , height(other.height) {

        }

        explicit Size(const Vector2<T>& point)
            : widht(point.x)
            , height(point.y) {

        }

        Size& operator= (const Size& other) {
            setSize(other.width, other.height);
            return *this;
        }

        Size& operator= (const Vector2<T>& point) {
            setSize(point.x, point.y);
            return *this;
        }

        Size operator+(const Size& right) const {
            return Size(width + right.width, height + right.height);
        }

        Size operator-(const Size& right) const {
            return Size(width - right.width, height - right.height);
        }

        Size operator*(T value) const {
            return Size(width * value, height * value);
        }

        Size operator/(T value) const {
            return Size(width / (double)value, height / (double)value);
        }

        void setSize(T _width, T _height) {
            width = _width;
            height = _height;
        }

        bool equals(const Size& target) const {
            return MATHEQUALS<T>(width, target.width) && MATHEQUALS<T>(height, target.height);
        }

    public:
        T width;
        T height;
    };

    typedef Size<float> Sizef;
    static const Sizef SizeZERO(0.0f, 0.0f);
}

#endif // SIZE_H
