#include "Functions.h"

#include <cmath>
#include <locale>
#include <codecvt>
#include <bitset>
#include <sstream>
#include <fstream>
#include <algorithm>

// Number manipulation
int absv(int v)
{
    int mask = v >> 31;
    return (mask ^ v) - mask;
}

bool is_equal_f(float f1, float f2, float epsilon)
{
    return (fabsf(f1 - f2) <= epsilon);
}

bool is_equal_d(double d1, double d2, double epsilon)
{
    return (fabs(d1 - d2) <= epsilon);
}

float sin_norm(float f)
{
    return sin((f * Math::PI) - (Math::PI * 0.5f)) * 0.5f + 0.5f;
}

float cos_norm(float f)
{
    return cos((f - 1.0f) * Math::PI) * 0.5f + 0.5f;
}

float cos_norm_scaled(float f, float scale)
{
    float so2 = scale * 0.5f;
    return cos((f - 1.0f) * acosf(1.0f - 2.0f / scale)) * so2 + 1.0f - so2;
    //return cos((f * Math::PI - Math::PI) * scale) * 0.5f + 0.5f;
}

float safe_sqrt(float f)
{
    if (f >= 0.0f)
        return sqrt(f);
    else
        return -sqrt(-f);
}

// Vectors
std::pair<float, float> point_as_vectors(Pos2D<float> v1, Pos2D<float> v2, Pos2D<float> point)
{
    float a = cross_product(v2, point) / cross_product(v2, v1);
    float b = cross_product(v1, point) / cross_product(v1, v2);
    return std::pair<float, float>(a, b);
}

// Geometry
int distance_manh(Pos2D<int> pos1, Pos2D<int> pos2)
{
    return absv(pos1.x - pos2.x) + absv(pos1.y - pos2.y);
}

// Complex geometry
float direction(Pos2D<float> startPos, Pos2D<float> endPos)
{
    return 0.0f;
}

Pos2D<float> point_in_direction(Pos2D<float> originPos, float direction, float distance)
{
    Pos2D<float> vec = { distance, 0.0f };
    vec = point_rotated_by({ 0.0f, 0.0f }, vec, direction);
    originPos += vec;

    return originPos;

    // REEEEEEEEEEEEEEEEE this shit is useless
    if (is_equal_f(direction, Math::PI * 0.5f, 0.001f)) {
        originPos.y -= distance;
        return originPos;
    }
    else if (is_equal_f(direction, Math::PI * 1.0f, 0.001f)) {
        originPos.x -= distance;
        return originPos;
    }
    else if (is_equal_f(direction, Math::PI * 1.5f, 0.001f)) {
        originPos.y += distance;
        return originPos;
    }
    else if (is_equal_f(direction, Math::PI * 2.0f, 0.001f)) {
        originPos.x += distance;
        return originPos;
    }
    else {
        float x = distance / sqrt(1.0f + pow(tanf(direction), 2));
        float y = distance / sqrt(1.0f + pow(1.0f / tanf(direction), 2));
        //float x = 0.0f;
        //float y = 0.0f;

        if      (direction > Math::PI * 0.0f && direction < Math::PI * 0.5f) {
            // First Quarter
            originPos.x += x;
            originPos.y -= y;
            //return originPos;
        }
        else if (direction > Math::PI * 0.5f && direction < Math::PI * 1.0f) {
            // Second Quarter
            originPos.x -= x;
            originPos.y -= y;
            //return originPos;
        }
        else if (direction > Math::PI * 1.0f && direction < Math::PI * 1.5f) {
            // Third Quarter
            originPos.x -= x;
            originPos.y += y;
            //return originPos;
        }
        else if (direction > Math::PI * 1.5f && direction < Math::PI * 2.0f) {
            // Fourth Quarter
            originPos.x += x;
            originPos.y += y;
            //return originPos;
        }

        return originPos;
    }
}

Pos2D<float> point_rotated_by(Pos2D<float> rotationAxis, Pos2D<float> p, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    // Translate point back to origin
    p -= rotationAxis;

    // Totate point
    float xnew = p.x * c + p.y * s;
    float ynew = -p.x * s + p.y * c;

    // translate point back:
    p.x = xnew + rotationAxis.x;
    p.y = ynew + rotationAxis.y;

    return p;
}

// Bounds

bool add_bounds(Pos2D<int> &pos, const RECT &bounds)
{
    bool inBounds = true;
    // x
    if        (pos.x < bounds.left) {
        pos.x = bounds.left;
        inBounds = false;
    }
    else if (pos.x > bounds.right) {
        pos.x = bounds.right;
        inBounds = false;
    }
    // y
    if        (pos.y < bounds.top) {
        pos.y = bounds.top;
        inBounds = false;
    }
    else if (pos.y > bounds.bottom) {
        pos.y = bounds.bottom;
        inBounds = false;
    }

    return inBounds;
}

bool add_bounds(int &value, const int &min_v, const int &max_v)
{
    if        (value < min_v) {
        value = min_v;
        return false;
    }
    else if (value > max_v) {
        value = max_v;
        return false;
    }

    return true;
}

bool add_bounds(float &value, const float &min_v, const float &max_v)
{
    if (value < min_v) {
        value = min_v;
        return false;
    }
    else if (value > max_v) {
        value = max_v;
        return false;
    }

    return true;
}

bool check_bounds(const Pos2D<int> &pos, const RECT &bounds)
{
    // x
    if (pos.x < bounds.left)
        return false;
    if (pos.x > bounds.right)
        return false;
    // y
    if (pos.y < bounds.top)
        return false;
    if (pos.y > bounds.bottom)
        return false;

    return true;
}

bool check_bounds(const int &value, const int &min_v, const int &max_v)
{
    if (value < min_v)
        return false;
    if (value > max_v)
        return false;

    return true;
}

bool check_bounds(const float &value, const float &min_v, const float &max_v)
{
    if (value < min_v)
        return false;
    if (value > max_v)
        return false;

    return true;
}
