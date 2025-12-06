#pragma once

#include <vector>
#include <string>

namespace zwnd
{
    enum class CursorIcon
    {
        APP_STARTING,
        ARROW,
        CROSS,
        HAND,
        HELP,
        IBEAM,
        NO,
        SIZE_ALL,
        SIZE_NESW,
        SIZE_NS,
        SIZE_NWSE,
        SIZE_WE,
        UP_ARROW,
        WAIT
    };
    constexpr std::vector<std::pair<int64_t, std::wstring>> CursorIconValueProxySelectionValues()
    {
        return {
            { (int64_t)CursorIcon::APP_STARTING, L"Start" },
            { (int64_t)CursorIcon::ARROW, L"Arrow" },
            { (int64_t)CursorIcon::CROSS, L"Cross" },
            { (int64_t)CursorIcon::HAND, L"Hand" },
            { (int64_t)CursorIcon::HELP, L"Help" },
            { (int64_t)CursorIcon::IBEAM, L"IBeam" },
            { (int64_t)CursorIcon::NO, L"No" },
            { (int64_t)CursorIcon::SIZE_ALL, L"Size all" },
            { (int64_t)CursorIcon::SIZE_NESW, L"Size NESW" },
            { (int64_t)CursorIcon::SIZE_NS, L"Size NS" },
            { (int64_t)CursorIcon::SIZE_NWSE, L"Size NWSE" },
            { (int64_t)CursorIcon::SIZE_WE, L"Size WE" },
            { (int64_t)CursorIcon::UP_ARROW, L"Up arrow" },
            { (int64_t)CursorIcon::WAIT, L"Wait" }
        };
    }
}