#define D2D_INPUT_COUNT 1
#define D2D_INPUT0_SIMPLE
#include "d2d1effecthelpers.hlsli"

cbuffer constants : register(b0)
{
    float r : packoffset(c0.x);
    float g : packoffset(c0.y);
    float b : packoffset(c0.z);
    float a : packoffset(c0.w);
};

D2D_PS_ENTRY(D2D_ENTRY)
{
    float4 color = D2DGetInput(0);
    color.x *= r;
    color.y *= g;
    color.z *= b;
    color.w *= a;
    return color;
}