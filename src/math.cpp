#pragma once
#include <algorithm>
#include <sstream>
#include <string>
#include <math.h>
namespace game 
{
    template <class T>
    struct Vec2
    {
        T x;
        T y;

        Vec2()
        {
            this->x = 0;
            this->x = 0;
        }

        Vec2(T x, T y)
        {
            this->x = x;
            this->y = y;
        }
        bool operator==(const Vec2 &rhs) const 
        {
            return this->x == rhs.x && this->y == rhs.y; 
        }
        bool operator<(const Vec2 &rhs) const 
        {
            return this->mag() < rhs.mag();
        }

        std::string as_str()
        {
            std::stringstream st;
            st << "Vec2(" << this->x << ", " << this->y << ")"; 
            return st.str();
        }

        const Vec2<T> add(const Vec2<T> other) const
        {
            return Vec2(this->x + other.x, this->y + other.y);
        }
        
        const Vec2<T> sub(const Vec2<T> other) const
        {
            return Vec2(this->x - other.x, this->y - other.y);
        }

        Vec2<T> mul(Vec2<T> other) const
        {
            return Vec2(this->x * other.x, this->y * other.y);
        }

        Vec2<T> div(Vec2<T> other) const
        {
            return Vec2(this->x / other.x, this->y / other.y);
        }

        Vec2<T> mul(T other) const
        {
            return Vec2(this->x * other, this->y * other);
        }

        Vec2<T> div(T other) const
        {
            return Vec2(this->x / other, this->y / other);
        }

        Vec2<T> neg() const
        {
            return Vec2(-x, -y);
        }

        T mag() const
        {
            return sqrt(x*x + y*y);
        }

        Vec2<T> norm() const
        {
            auto mag = this->mag();
            if (mag != 0)
                return this->div(mag);
            return Vec2(0, 0);
        }
    };

    template <class T>
    struct Vec2Comparator
    {
        bool operator()(const Vec2<T> &c1, const Vec2<T> &c2) const
        {
            return c1 < c2;
        }
    };

    template <class T>
    struct Rect
    {
        Vec2<T> pos;
        Vec2<T> size;

        Rect(T x, T y, T width, T height)
        {
            this->pos = Vec2<T>(x, y);
            this->size = Vec2<T>(width, height);
        }

        Rect<T> diff(Rect<T> other)
        {
            auto x = std::max(this->pos.x, other.pos.x);
            auto y = std::max(this->pos.y, other.pos.y);
            return Rect<T>(x,
                           y,
                           std::min(this->pos.x+this->size.x, other.pos.x+other.size.x)-x,
                           std::min(this->pos.y+this->size.y, other.pos.y+other.size.y)-y);
        }

        Rect(Vec2<T> pos, Vec2<T> size)
        {
            this->pos = pos;
            this->size = size;
        }

        bool intersects(Rect<T> other)
        {
            return pos.x + size.x > other.pos.x &&
                   pos.x < other.pos.x + other.size.x &&
                   pos.y + size.y > other.pos.y &&
                   pos.y < other.pos.y + other.size.y;
        }

        Rect<T> at(Vec2<T> pos)
        {
            return Rect(this->pos.add(pos), this->size);
        }
    };
}

namespace std 
{
    template <class T>
    struct hash<game::Vec2<T>>
    {
        std::size_t operator()(const game::Vec2<T> &v) const 
        {
            return v.x + v.y;
        }
    };
}
