#pragma once

#include <string>
#include <vector>
#include <optional>

#include "json.hpp"

#include "Helper/Time.h"

namespace osu
{
    enum class GameMode
    {
        STD = 0,
        TAIKO = 1,
        CATCH = 2,
        MANIA = 3
    };

    enum class RankedStatus
    {
        UNKNOWN = 0,
        UNSUBMITTED = 1,
        PENDING = 2,
        UNUSED = 3,
        RANKED = 4,
        APPROVED = 5,
        QUALIFIED = 6
    };

    struct Beatmap
    {
        TimePoint firstObjectTime = 0;
        TimePoint currentTime = 0;
        TimePoint lastObjectTime = 0;
        Duration mp3Length = 0;
        std::string mapId;
        std::string setId;
        std::string md5;
        RankedStatus rankedStatus = RankedStatus::UNKNOWN;
        std::string artist;
        std::string title;
        std::string mapper;
        std::string difficultyName;

        float AR = 0;
        float CS = 0;
        float OD = 0;
        float HP = 0;
        float currentSR = 0;
        float minBPM = 0;
        float maxBPM = 0;
        float baseAR = 0;
        float baseCS = 0;
        float baseOD = 0;
        float baseHP = 0;
        float baseSR = 0;

        std::string fullPath;
        std::string folderPath;
        std::string filePath;
        std::string backgroundFilename;
        std::string audioFilename;
    };

    struct MenuState
    {
        int state = 0;
        std::string skinFolder;
        GameMode gameMode = GameMode::STD;
        int isChatEnabled = 0;
        Beatmap beatmap;
        int modNumber = 0;
        std::string modString;
        int ppFor100 = 0;
        int ppFor99 = 0;
        int ppFor98 = 0;
        int ppFor97 = 0;
        int ppFor96 = 0;
        int ppFor95 = 0;
        std::vector<float> strains;
    };

    enum class Team
    {
        SOLO = 0,
        BLUE = 1,
        RED = 2
    };

    struct LeaderboardItem
    {
        std::string username;
        int score = 0;
        int combo = 0;
        int maxCombo = 0;
        std::string mods;
        int count300 = 0;
        int count100 = 0;
        int count50 = 0;
        int countMiss = 0;
        Team team = Team::SOLO;
        int position = 0;
        int isPassing = 0;
    };

    struct Leaderboard
    {
        int hasLeaderboard = 0;
        std::optional<LeaderboardItem> currentPlayer = std::nullopt;
        std::vector<LeaderboardItem> players;
    };

    struct GameplayState
    {
        GameMode gameMode = GameMode::STD;
        std::string playerName;
        int score = 0;
        float accuracy = 0;
        int currentCombo = 0;
        int maxCombo = 0;
        float normalCurrentHP = 0;
        float smoothedCurrentHP = 0;
        int count300 = 0;
        int count200 = 0;
        int countGeki = 0;
        int count100 = 0;
        int countKatu = 0;
        int count50 = 0;
        int countMiss = 0;
        int countSliderbreak = 0;
        std::string currentGrade;
        std::string maxGradeThisPlay;
        float unstableRate = 0;
        std::vector<int> hitErrorArray;
        float currentPP = 0;
        int PPforFC = 0;
        int maxPPForThisPlay = 0;
        Leaderboard leaderboard;
    };

    struct GameState
    {
        std::string client;
        std::string server;

        struct State
        {
            int number;
            std::string name;
        };
        State state;

        struct Session
        {
            int64_t playTime;
            int64_t playCount;
        };
        Session session;

        struct Settings
        {
            struct Mode
            {
                int number;
                std::string name;
            };
            Mode mode;
        };
        Settings settings;

        struct Profile
        {
            struct BanchoStatus
            {
                int number;
                std::string name;
            };
            BanchoStatus banchoStatus;

            int64_t id;
            std::string name;

            struct Mode
            {
                int64_t number;
                std::string name;
            };
            Mode mode;
        };
        Profile profile;

        struct Beatmap
        {
            struct Time
            {
                TimePoint live;
                TimePoint firstObject;
                TimePoint lastObject;
                Duration mp3Length;
            };
            Time time;

            struct Status
            {
                int number;
                std::string name;
            };
            Status status;

            std::string id;

            struct Mode
            {
                int number;
                std::string name;
            };
            Mode mode;
        };
        Beatmap beatmap;

        struct Play
        {
            std::string playerName;

            struct Mode
            {
                int number;
                std::string name;
            };
            Mode mode;

            int64_t score;
            float accuracy;

            struct Hits
            {
                int count300;
                int count100;
                int count50;
                int countMiss;
            };
            Hits hits;

            struct Combo
            {
                int current;
            };
            Combo combo;

            struct Mods
            {
                float rate;
            };
            Mods mods;

            struct PP
            {
                float current;
            };
            PP pp;

            float unstableRate;
        };
        Play play;

        struct Performance
        {
            struct Graph
            {
                std::vector<float> aim;
                std::vector<float> aimNoSliders;
                std::vector<float> flashlight;
                std::vector<float> speed;
                std::vector<int64_t> xaxis;
            };
            Graph graph;
        };
        Performance performance;

        struct ResultsScreen
        {
            int64_t scoreId;
            std::string playerName;

            struct PP
            {
                float current;
            };
            PP pp;

            std::string createdAt;
        };
        ResultsScreen resultsScreen;
    };

    void ParseJson(GameState& state, const std::string& inputStr);
}