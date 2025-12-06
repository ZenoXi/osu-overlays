#pragma once

#include "Overlays/Overlay.h"

#include "UICore/Components/Base/Label.h"
#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Toggle.h"
#include "UICore/Components/Base/NumberInput.h"
#include "LeaderboardItem.h"

#include "OsuDataProvider/DataProviderView.h"
#include "OsuWebApi/Response/WebResponse.h"
#include "OsuWebApi/Response/PlayerData.h"
#include "OsuWebApi/Response/LeaderboardPage.h"
#include "OsuWebApi/Response/BeatmapUserScore.h"

namespace zcom
{
    class RTLeaderboardOverlayComponent : public Panel
    {
        DEFINE_COMPONENT(RTLeaderboardOverlayComponent, Panel)
        DEFAULT_DESTRUCTOR(RTLeaderboardOverlayComponent)
    protected:
        void Init(std::shared_ptr<const Overlay> overlay);

    private:
        void _OnUpdate() override;

        std::shared_ptr<const Overlay> _overlay;

        std::unique_ptr<DataProviderView> _dataProviderView;

        std::unique_ptr<FlexPanel> _mainPanel;
        std::unique_ptr<Panel> _leaderboardPanel;
        std::unique_ptr<LeaderboardItem> _userLeaderboardItem;
        std::vector<std::unique_ptr<LeaderboardItem>> _leaderboardItems;
        Value<float> SCALE = 1.33f;
        Value<Size> _itemSize = Size{ -10, 60 };
        Value<int> _itemSpacing = 5;
        Value<int> _visibleItemCount = 5;
        bool _visibleItemCountChanged = false;
        bool _scaleChanged = false;

        Value<Color> _playerBackgroundColor = Color();
        Value<Color> _nonPlayerBackgroundColor = Color();
        Value<Color> _usernameTextColor = Color();
        Value<Color> _ppTextColor = Color();
        Value<Color> _rankTextColor = Color();

        std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> _configValueChangedEventSubscription = nullptr;

        int _currentGlobalOffset = 0;

        bool _movingItems = false;
        int _startGlobalOffset = 0;
        int _targetGlobalOffset = 0;
        TimePoint _moveStart = TimePoint(0);
        Duration _moveDuration = Duration(500, MILLISECONDS);

        bool _movingIn = false;
        bool _movingOut = false;
        bool _leaderboardHidden = false;
        TimePoint _moveXStart = TimePoint(0);
        Duration _moveXDuration = Duration(500, MILLISECONDS);

        std::optional<std::string> _userId;
        std::optional<std::string> _countryCode;
        int64_t _previousResultId = -1;
        int _previousState = -1;
        bool _playing = false;
        std::string _playMode = "osu";
        bool _playingPlayerUnresolvableWarningShown = false;
        bool _dataProviderNotEnabledWarningShown = false;
        bool _loadingLeaderboardPage = false;

        bool _showIntroNotification = false;


        std::optional<int64_t> _scoreCount = std::nullopt;
        float _sessionBonusPPIncrease = 0.0f;
        int _sessionScoreCount = 0;

        struct _PlayerData
        {
            std::string userId;
            std::string username;
            float pp;
            int scoreCount;
            std::string mode;

            struct _Score
            {
                int rank;
                float pp;
                float ppWeighted;
                std::string mapId;
            };
            std::vector<_Score> scores;

            std::vector<float> lastPPGains;
        };

        struct _MapPlayerScore
        {
            float pp;
        };

        struct _User
        {
            std::string username;
            float pp = 0;
            int64_t globalRank = 0; // user rank as retrieved from the api
            int64_t indexRank = 0; // user index in the leaderboard when sorted by pp
        };

        struct _LeaderboardPage
        {
            std::vector<_User> users;
            int64_t pageNumber = 0;
        };

        std::optional<_PlayerData> _loggedInPlayerData = std::nullopt;
        std::optional<_PlayerData> _playingPlayerData = std::nullopt;
        std::optional<_MapPlayerScore> _mapPlayerScore = std::nullopt;
        std::optional<int> _currentMapIndexInPlayerTopScores = std::nullopt;
        std::vector<_LeaderboardPage> _leaderboard;

        enum class _LoadStatus
        {
            NOT_LOADED,
            LOADING,
            LOADED
        };
        _LoadStatus _loggedInPlayerDataStatus = _LoadStatus::NOT_LOADED;
        _LoadStatus _playingPlayerDataStatus = _LoadStatus::NOT_LOADED;
        _LoadStatus _leaderboardStatus = _LoadStatus::NOT_LOADED;
        _LoadStatus _mapPlayerScoreStatus = _LoadStatus::NOT_LOADED;
        _LoadStatus _newScoreLoadStatus = _LoadStatus::NOT_LOADED;
        bool _extraPlayDataLoaded = false;

        int _previousRank = 0;
        int _currentRank = 0;
        float _currentTotalPP = 0.0f;

        std::unique_ptr<AsyncEventSubscription<void, webapi::resp::WebResponse<webapi::resp::PlayerData>>> _playerDataSubscription;
        std::unique_ptr<AsyncEventSubscription<void, webapi::resp::WebResponse<webapi::resp::PlayerData>>> _otherPlayerDataSubscription;
        std::unique_ptr<AsyncEventSubscription<void, webapi::resp::WebResponse<std::vector<webapi::resp::LeaderboardPage>>>> _initialLeaderboardSubscription;
        std::unique_ptr<AsyncEventSubscription<void, webapi::resp::WebResponse<webapi::resp::BeatmapUserScore>>> _mapPlayerScoreSubscription;
        std::unique_ptr<AsyncEventSubscription<void, webapi::resp::WebResponse<webapi::resp::LeaderboardPage>>> _leaderboardPageSubscription;
        std::unique_ptr<AsyncEventSubscription<void, std::optional<_PlayerData>>> _evaluatedScoreSubscription;

        void _UpdateCurrentPlayData(const osu::GameState& state);
        void _UpdateLeaderboardItems();
        void _EvaluateNewScore(std::string mapId, std::string userId, int64_t scoreId);

        void _ShowLeaderboard();
        void _HideLeaderboard();
        _User* _GetUserByIndexRank(int indexRank);
        std::unique_ptr<LeaderboardItem> _CreateLeaderboardItemForRank(int indexRank);
    };
}