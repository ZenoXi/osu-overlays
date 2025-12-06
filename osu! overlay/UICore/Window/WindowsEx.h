#pragma once

// Windows 7 and up
//#define _WIN32_WINNT 0x0601
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
//#ifndef NOMINMAX
#define NOMINMAX
//#endif

#include <Windows.h>
#include <windowsx.h>

#ifdef _DEBUG
#include <cassert>
#define HR(expression) assert(S_OK == (expression))
#else
#define HR(expression) expression
#endif

#include <string>
#include <optional>

inline std::optional<std::wstring> ToWinErrorString(DWORD errorCode)
{
    LPTSTR errorText = NULL;

    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&errorText,
        0,
        NULL
    );

    if (NULL != errorText)
    {
        std::wstring str(errorText);
        LocalFree(errorText);
        return str;
    }

    return std::nullopt;
}