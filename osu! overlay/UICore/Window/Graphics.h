#pragma once

#include "WindowsEx.h"

#pragma comment( lib,"d2d1.lib" )
#pragma comment( lib,"d3d11.lib" )
#pragma comment( lib,"dxguid.lib" )
#pragma comment( lib,"dwrite.lib" )
#include <d2d1_1.h>
#include <d3d11_1.h>
#include <d2d1effects.h>
#include <d2d1effects_2.h>
#include "Dwrite.h"

#include <vector>
#include <string>

struct Graphics
{
    ID2D1DeviceContext* target = nullptr;
    ID2D1Factory1* factory = nullptr;
    std::vector<std::pair<IUnknown**, std::string>>* refs = nullptr;
};

inline bool operator==(const D2D1_COLOR_F& cl, const D2D1_COLOR_F& cr)
{
    return cl.a == cr.a
        && cl.b == cr.b
        && cl.g == cr.g
        && cl.r == cr.r;
}