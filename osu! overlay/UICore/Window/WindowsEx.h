#pragma once

// Windows 7 and up
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <windowsx.h>

#ifdef _DEBUG
#include <cassert>
#define HR(expression) assert(S_OK == (expression))
#else
#define HR(expression) expression
#endif