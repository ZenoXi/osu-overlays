#pragma once

#include "Helper/ConfigValue.h"

struct RTLeaderboardConfig
{
    inline static const ConfigValue<std::wstring> LAYOUT_STRING = ConfigValue<std::wstring>(L"rtLeaderboard.layoutString", L"0|250|300|0|130|0|0");
    inline static const ConfigValue<bool> USE_SPECIFIC_COUNTRY = ConfigValue<bool>(L"rtLeaderboard.useCountry", false);
    inline static const ConfigValue<std::wstring> COUNTRY_CODE = ConfigValue<std::wstring>(L"rtLeaderboard.countryCode", L"US");
    inline static const ConfigValue<bool> FRIENDS_ONLY = ConfigValue<bool>(L"rtLeaderboard.friendsOnly", false);
    inline static const ConfigValue<bool> USE_OTHER_USER = ConfigValue<bool>(L"rtLeaderboard.useOtherUser", false);
    inline static const ConfigValue<std::wstring> OTHER_USER_ID = ConfigValue<std::wstring>(L"rtLeaderboard.otherUserId", L"");
    inline static const ConfigValue<float> UI_SCALE = ConfigValue<float>(L"rtLeaderboard.uiScale", 1.0f);
    inline static const ConfigValue<int> PLAYER_BACKGROUND_COLOR = ConfigValue<int>(L"rtLeaderboard.playerBackgroundColor", 0xBF4C4733);
    inline static const ConfigValue<int> NON_PLAYER_BACKGROUND_COLOR = ConfigValue<int>(L"rtLeaderboard.nonPlayerBackgroundColor", 0xBF303030);
    inline static const ConfigValue<int> USERNAME_TEXT_COLOR = ConfigValue<int>(L"rtLeaderboard.usernameTextColor", 0xFFD0D0D0);
    inline static const ConfigValue<int> PP_TEXT_COLOR = ConfigValue<int>(L"rtLeaderboard.ppTextColor", 0xFFD0D0D0);
    inline static const ConfigValue<int> RANK_TEXT_COLOR = ConfigValue<int>(L"rtLeaderboard.rankTextColor", 0xFF808080);

    inline static const ConfigValue<bool> INTRO_NOTIFICATION_SHOWN = ConfigValue<bool>(L"rtLeaderboard.introNotificationShown", false);
};