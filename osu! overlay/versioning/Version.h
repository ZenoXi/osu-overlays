#pragma once

#include "UICore/Helper/StringHelper.h"
#include <optional>

class VersionTag
{
    int _major = 0;
    int _minor = 0;
    int _patch = 0;

public:
    constexpr VersionTag() {}
    constexpr VersionTag(int major, int minor, int patch)
    {
        _major = major;
        _minor = minor;
        _patch = patch;
    }

    static std::optional<VersionTag> Parse(std::string tagString)
    {
        try
        {
            std::array<std::string, 3> tagParts;
            split_str(tagString, tagParts, '.');

            int major = std::stoi(tagParts[0]);
            int minor = std::stoi(tagParts[1]);
            int patch = std::stoi(tagParts[2]);
            return VersionTag(major, minor, patch);
        }
        catch (...)
        {
            return std::nullopt;
        }
    }

    int GetMajorVersion() const
    {
        return _major;
    }

    int GetMinorVersion() const
    {
        return _minor;
    }

    int GetPatchVersion() const
    {
        return _patch;
    }

    std::string ToString() const
    {
        return std::to_string(_major) + '.' + std::to_string(_minor) + '.' + std::to_string(_patch);
    }

    auto operator<=>(const VersionTag&) const = default;
};

constexpr VersionTag OVERLAY_ENGINE_VERSION = VersionTag(3, 0, 2);