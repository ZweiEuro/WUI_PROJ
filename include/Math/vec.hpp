
#pragma once
#include <cmath>

namespace WUI
{
    template <typename T>
    class vec2
    {
    public:
        T x;
        T y;

        vec2(T x = 0, T y = 0) : x(x), y(y)
        {
        }

        template <typename T2>
        vec2 operator*(T2 scale)
        {
            return {(T)(x * scale), (T)(y * scale)};
        }

        template <typename T2>
        vec2 operator/(T2 divisor)
        {
            return {(T)((double)x / (double)divisor), (T)((double)y / (double)divisor)};
        }

        /*vec2 operator*(float scale)
        {
          return {x * scale, y * scale};
        }*/

        vec2 operator+(const vec2 other) const
        {
            return {x + other.x, y + other.y};
        }

        vec2 operator+=(const vec2 other)
        {

            x += other.x;
            y += other.y;

            return {x, y};
        }

        vec2 operator-(const vec2 other) const
        {
            return {x - other.x, y - other.y};
        }

        bool operator==(const vec2 other) const
        {
            return x == other.x && y == other.y;
        }

        bool operator!=(const vec2 other) const
        {
            return !(this->operator==(other));
        }

        template <typename T2>
        operator vec2<T2>() const
        {
            return vec2<T2>((T2)x, (T2)y);
        }

        float mag() const
        {
            return std::sqrt(x * x + y * y);
        }

        float dot(vec2 other) const
        {
            return x * other.x + y * other.y;
        }

        vec2<float> dir() const // i.e. normalize
        {
            return {((float)x) / mag(), ((float)y) / mag()};
        }
    };

};
using vec2f = WUI::vec2<float>;
using vec2i = WUI::vec2<int>;