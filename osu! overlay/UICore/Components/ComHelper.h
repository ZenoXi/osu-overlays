#pragma once

namespace zcom
{
    struct RECT_F
    {
        float left;
        float top;
        float right;
        float bottom;

        bool operator!=(const RECT_F& other)
        {
            return
                left != other.left ||
                top != other.top ||
                right != other.right ||
                bottom != other.bottom;
        }

        bool operator==(const RECT_F& other)
        {
            return
                left == other.left &&
                top == other.top &&
                right == other.right &&
                bottom == other.bottom;
        }
    };
}