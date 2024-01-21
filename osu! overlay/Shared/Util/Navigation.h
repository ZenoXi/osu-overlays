#pragma once

#ifndef POSITION_H
#define POSITION_H

#include "Constants.h"

#include <cmath>
#include <unordered_map>

enum Direction
{
    NO_DIRECTION = -1,
    UP,
    RIGHT,
    DOWN,
    LEFT
};

template<class T>
class Pos2D
{
public:
    T x;
    T y;

private:
    bool invalid = false;
public:

    Pos2D() : x(0), y(0) {}
    Pos2D(T x, T y, bool iv = false) : x(x), y(y), invalid(iv) {}
    Pos2D(T s, bool iv = false) : x(s), y(s), invalid(iv) {}
    Pos2D(const Pos2D &c)
    {
        this->x = c.x;
        this->y = c.y;
        this->invalid = c.is_invalid();
    }

    bool is_invalid() const
    {
        return invalid;
    }

    void move(Direction dir, T amount)
    {
        switch (dir)
        {
        case UP:
            y -= amount;
            break;
        case RIGHT:
            x += amount;
            break;
        case DOWN:
            y += amount;
            break;
        case LEFT:
            x -= amount;
            break;
        }
    }

    int index(int width) const
    {
        return floor(y) * width + floor(x);
    }

    int index(int width, Direction dir, T dist) const
    {
        switch (dir)
        {
            case UP:
                return floor(y - dist) * width + floor(x);
            case RIGHT:
                return floor(y) * width + floor(x + dist);
            case DOWN:
                return floor(y + dist) * width + floor(x);
            case LEFT:
                return floor(y) * width + floor(x - dist);
        }
    }

    float vector_length() const
    {
        return sqrt(x * x + y * y);
    }
    float vector_length_sqr() const
    {
        return x * x + y * y;
    }

    float rotation() const
    {
        return fmod(atan2(y, -x), Math::PI) + Math::PI;
    }

    Pos2D<T> perpendicularL() const
    {
        return Pos2D<T>(y, -x);
    }

    Pos2D<T> perpendicularR() const
    {
        return Pos2D<T>(-y, x);
    }

    void set_vector_length(float length)
    {
        float ratio = length / vector_length();
        x = static_cast<T>(x * ratio);
        y = static_cast<T>(y * ratio);
    }

    Pos2D<T> of_length(float length)
    {
        Pos2D<T> vec;
        float ratio = length / vector_length();
        vec.x = static_cast<T>(x * ratio);
        vec.y = static_cast<T>(y * ratio);
        return vec;
    }

    Direction vdirection() const
    {
        if (x == 0 && y == 0)
            return NO_DIRECTION;

        if (abs(x) >= abs(y))
            if (x > 0)
                return RIGHT;
            else
                return LEFT;
        else
            if (y > 0)
                return UP;
            else
                return DOWN;
    }

    template<typename T_>
    Pos2D<T_> cast()
    {
        return Pos2D<T_>(static_cast<T_>(x), static_cast<T_>(y), invalid);
    }

    Pos2D operator= (const Pos2D &p)
    {
        this->x = p.x;
        this->y = p.y;
        this->invalid = p.is_invalid();
        return *this;
    }

    Pos2D operator-() const
    {
        return Pos2D
        (
            -this->x,
            -this->y,
            this->invalid
        );
    }

    Pos2D operator+ (const Pos2D &p) const
    {
        return Pos2D
        (
            this->x + p.x,
            this->y + p.y,
            false
        );
    }

    Pos2D operator- (const Pos2D &p) const
    {
        return Pos2D
        (
            this->x - p.x,
            this->y - p.y,
            false
        );
    }

    void operator+= (const Pos2D &p) {
        this->x += p.x;
        this->y += p.y;
    }

    void operator-= (const Pos2D &p) {
        this->x -= p.x;
        this->y -= p.y;
    }

    template<typename T_>
    Pos2D operator* (T_ t) const
    {
        return Pos2D
        (
            this->x * t,
            this->y * t,
            false
        );
    }

    template<typename T_>
    Pos2D operator/ (T_ t) const
    {
        return Pos2D
        (
            this->x / t,
            this->y / t,
            false
        );
    }

    Pos2D operator% (int t) const
    {
        return Pos2D
        (
            this->x % t,
            this->y % t,
            false
        );
    }
};

template<typename T>
inline bool operator ==(const Pos2D<T> &p1, const Pos2D<T> &p2)
{
    return (p1.x == p2.x && p1.y == p2.y);
}

template<typename T>
inline bool operator< (const Pos2D<T> &p1, const Pos2D<T> &p2)
{
    return (p1.x < p2.x || p1.y < p2.y);
}

template<typename T>
inline bool operator> (const Pos2D<T> &p1, const Pos2D<T> &p2)
{
    return (p1.x > p2.x || p1.y > p2.y);
}

void rotate_direction(Direction &dir, int t);

extern std::unordered_map<std::string, Direction> directionMap;
extern std::unordered_map<Direction, Pos2D<int>> directionVectorMap;

#endif // POSITION_H
