#include "App.h" // App.h must be included first
#include "SharedContext.h"
#include "Window/Window.h"
#include "RTLeaderboardOverlayComponent.h"
#include "RTLeaderboardConfig.h"
#include "LeaderboardCountries.h"

#include "UICore/Fonts/FontLoader.h"

#include "OsuWebApi/WebApiConfig.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Util/Http.h"
#include "Shared/Util/JsonParser.h"
#include "Shared/Util/Streams.h"

#include "notification/StandardNotificationTemplate.h"

void zcom::RTLeaderboardOverlayComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    Panel::Init();

    _overlay = overlay;
    _dataProviderView = std::make_unique<DataProviderView>(this);

    if (!_dataProviderView->running_)
    {
        StandardNotificationTemplate notif;
        notif.showDuration = Duration(10, SECONDS);
        notif.type = StandardNotificationTemplate::NotificationType::WARNING;
        notif.title = L"Data provider not enabled";
        notif.text = L"Real-time leaderboard will not be shown until osu! data provider is enabled";
        _scene->GetApp()->Shared<SharedContext*>()->notificationService.ShowNotification(notif.ToNotificationInfo());
        _showIntroNotification = false;
        _scene->GetApp()->config.SetIntValue(RTLeaderboardConfig::INTRO_NOTIFICATION_SHOWN.name, true);
    }

    _scene->GetWindow()->GetTextRenderContext()->AddFontsFromFile({
        L"Resources/Fonts/Nunito-VariableFont_wght.ttf",
        L"Resources/Fonts/Nunito-Italic-VariableFont_wght.ttf"
    });

    _mainPanel = Create<FlexPanel>(FlexDirection::DOWN);
    _mainPanel->parentSize = { 1.0f, 1.0f };
    _mainPanel->visible = false;

    _UpdateVariableConfigValues();

    bool useOtherUser = _scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::USE_OTHER_USER, Config::ADD_AND_SAVE_IF_MISSING);
    if (useOtherUser)
        _userId = wstring_to_string(_scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::OTHER_USER_ID, Config::ADD_AND_SAVE_IF_MISSING));
    bool useCountry = _scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::USE_SPECIFIC_COUNTRY, Config::ADD_AND_SAVE_IF_MISSING);
    if (useCountry)
        _countryCode = wstring_to_string(_scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::COUNTRY_CODE, Config::ADD_AND_SAVE_IF_MISSING));

    _configValueChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        ExecuteSynchronously([=]() {
            _UpdateVariableConfigValues();
            ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::LAYOUT_STRING), this);
        });
    });
    SCALE.Subscribe([=](float) { _scaleChanged = true; }).Detach();
    ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(RTLeaderboardConfig::LAYOUT_STRING, Config::ADD_AND_SAVE_IF_MISSING), this);

    _showIntroNotification = !_scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::INTRO_NOTIFICATION_SHOWN, Config::ADD_AND_SAVE_IF_MISSING);

    _itemSize.ComputedFrom([](float scale) { return Size{ -10, int(60 * scale) }; }, SCALE);
    _itemSpacing.ComputedFrom([](float scale) { return int(5 * scale); }, SCALE);

    std::wstring leaderboardString = L"Global leaderboard";
    if (_countryCode)
    {
        auto fullNameOpt = LeaderboardCountries::FindFullNameFromCode(string_to_wstring(_countryCode.value()));
        if (fullNameOpt)
            leaderboardString = fullNameOpt.value() + L" leaderboard";
    }
    auto leaderboardLabel = Create<Label>(leaderboardString);
    leaderboardLabel->parentSize = { 1.0f, 0.0f };
    leaderboardLabel->autoHeight = true;
    leaderboardLabel->padding.ComputedFrom([](float scale) { return RectF{ 5.0f * scale }; }, SCALE);
    leaderboardLabel->font = L"Nunito";
    leaderboardLabel->fontSize.ComputedFrom([](float fontSize, float scale) { return fontSize * scale; }, _titleLabelFontSize, SCALE);
    leaderboardLabel->yTextAlign = Alignment::CENTER;
    leaderboardLabel->wordWrapping = WordWrapping::WRAP;
    leaderboardLabel->visible.ComputedFrom([](bool visible) { return visible; }, _showTitleLabel);

    auto leaderboardPanelWrapper = Create<Panel>();
    leaderboardPanelWrapper->parentSize = { 1.0f, 0.0f };
    leaderboardPanelWrapper->SetProperty(FlexGrow());
    _visibleItemCount.ComputedFrom([](Size itemPanelSize, Size itemSize) {
        return itemPanelSize.height / (itemSize.height + 5);
    }, leaderboardPanelWrapper->size_, _itemSize);
    _visibleItemCount.Subscribe([=](int) { _visibleItemCountChanged = true; }).Detach();
    _leaderboardPanel = Create<Panel>();
    _leaderboardPanel->parentSize = { 1.0f, 0.0f };
    _leaderboardPanel->size.ComputedFrom([](Size itemSize, int itemSpacing, int visibleItemCount) {
        return Size{ itemSize.width, (itemSize.height + itemSpacing) * visibleItemCount };
    }, _itemSize, _itemSpacing, _visibleItemCount);
    leaderboardPanelWrapper->AddItem(_leaderboardPanel.get());

    _userLeaderboardItem = Create<LeaderboardItem>(&SCALE);
    _userLeaderboardItem->size.ComputedFrom([](Size size) { return size; }, _itemSize);
    _userLeaderboardItem->backgroundColor.ComputedFrom([](Color color) { return color; }, _playerBackgroundColor);
    _userLeaderboardItem->GetUsernameLabel()->fontColor.ComputedFrom([](Color color) { return color; }, _usernameTextColor);
    _userLeaderboardItem->GetPPLabel()->fontColor.ComputedFrom([](Color color) { return color; }, _ppTextColor);
    _userLeaderboardItem->GetRankLabel()->fontColor.ComputedFrom([](Color color) { return color; }, _rankTextColor);
    _userLeaderboardItem->zIndex = 10;

    _mainPanel->AddItem(std::move(leaderboardLabel));
    _mainPanel->AddItem(std::move(leaderboardPanelWrapper));
    
    AddItem(_mainPanel.get());
}

void zcom::RTLeaderboardOverlayComponent::_UpdateVariableConfigValues()
{
    SCALE = _scene->GetApp()->config.GetDoubleConfigValue(RTLeaderboardConfig::UI_SCALE);
    _showTitleLabel = _scene->GetApp()->config.GetDoubleConfigValue(RTLeaderboardConfig::SHOW_TITLE_LABEL);
    _titleLabelFontSize = _scene->GetApp()->config.GetDoubleConfigValue(RTLeaderboardConfig::TITLE_LABEL_FONT_SIZE);
    _playerBackgroundColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::PLAYER_BACKGROUND_COLOR));
    _nonPlayerBackgroundColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::NON_PLAYER_BACKGROUND_COLOR));
    _usernameTextColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::USERNAME_TEXT_COLOR));
    _ppTextColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::PP_TEXT_COLOR));
    _rankTextColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(RTLeaderboardConfig::RANK_TEXT_COLOR));
}

void zcom::RTLeaderboardOverlayComponent::_OnUpdate()
{
    Panel::_OnUpdate();

    if (_showIntroNotification)
    {
        StandardNotificationTemplate notif;
        notif.showDuration = Duration(15, SECONDS);
        notif.type = StandardNotificationTemplate::NotificationType::WARNING;
        notif.title = L"Bonus PP estimation";
        notif.text = L"Submit any score on a ranked map to estimate your bonus PP. Before estimation the gained PP value in the real-time leaderboard might be inaccurate";
        _scene->GetApp()->Shared<SharedContext*>()->notificationService.ShowNotification(notif.ToNotificationInfo());
        _showIntroNotification = false;
        _scene->GetApp()->config.SetIntValue(RTLeaderboardConfig::INTRO_NOTIFICATION_SHOWN.name, true);
    }

    std::optional<osu::GameState> stateOpt = _scene->GetApp()->Shared<SharedContext*>()->dataProvider.GetGameState();
    if (stateOpt)
    {
        osu::GameState state = stateOpt.value();

        bool inPlayState = state.state.number == 2;
        if (inPlayState)
        {
            if (!_playing)
            {
                _playing = true;
                _playMode = state.beatmap.mode.name == "osu" ? state.settings.mode.name : state.beatmap.mode.name;
                _playingPlayerData = std::nullopt;
                _playingPlayerDataStatus = _LoadStatus::NOT_LOADED;
                _playingPlayerUnresolvableWarningShown = false;
            }
        }
        else
        {
            _playing = false;
        }

        if (_loggedInPlayerDataStatus == _LoadStatus::NOT_LOADED)
        {
            if (_playing)
            {
                if (state.profile.id != -1)
                {
                    _playerDataSubscription = _scene->GetApp()->Shared<SharedContext*>()->webApi.GetPlayerData(std::to_string(state.profile.id), _playMode);
                    _loggedInPlayerDataStatus = _LoadStatus::LOADING;
                }
                else
                {
                    _loggedInPlayerData = _PlayerData{ "-1" };
                    _loggedInPlayerDataStatus = _LoadStatus::LOADED;
                }
            }
        }
        else if (_loggedInPlayerDataStatus == _LoadStatus::LOADING)
        {
            _playerDataSubscription->HandlePendingEvents([=](const webapi::resp::WebResponse<webapi::resp::PlayerData>& response) {
                if (response.status != 200 || !response.content.has_value())
                {
                    // Try again after delay
                    ExecuteSynchronously([=]() {
                        _loggedInPlayerDataStatus = _LoadStatus::NOT_LOADED;
                        _loggedInPlayerData = std::nullopt;
                    }, Duration(10, SECONDS));
                    return;
                }

                if (response.content->userId == std::to_string(state.profile.id))
                {
                    _PlayerData data{};
                    data.userId = response.content->userId;
                    data.username = response.content->username;
                    data.mode = response.content->mode;
                    data.pp = response.content->pp;
                    data.ppSumOfBestScores = 0.0f;
                    for (auto& score : response.content->scores)
                    {
                        data.scores.push_back({ 0, score.pp, score.ppWeighted, score.mapId });
                        data.ppSumOfBestScores += score.ppWeighted;
                    }
                    data.scoreCount = -1;

                    _loggedInPlayerData = data;
                    _loggedInPlayerDataStatus = _LoadStatus::LOADED;
                }
                else
                {
                    _loggedInPlayerDataStatus = _LoadStatus::NOT_LOADED;
                }
            });
        }
        else if (_loggedInPlayerDataStatus == _LoadStatus::LOADED)
        {
            if (_loggedInPlayerData->userId != std::to_string(state.profile.id) || (_playing && _playMode != _loggedInPlayerData->mode))
            {
                _loggedInPlayerData = std::nullopt;
                _loggedInPlayerDataStatus = _LoadStatus::NOT_LOADED;
            }
        }

        auto afterPlayingPlayerLoad = [=]() {
            _leaderboardStatus = _LoadStatus::NOT_LOADED;
            _mapPlayerScoreStatus = _LoadStatus::NOT_LOADED;
            _mapPlayerScore = std::nullopt;
            _currentMapIndexInPlayerTopScores = std::nullopt;
            _extraPlayDataLoaded = false;
        };

        if (_playingPlayerDataStatus == _LoadStatus::NOT_LOADED)
        {
            if (_playing)
            {
                if (_userId)
                {
                    _otherPlayerDataSubscription = _scene->GetApp()->Shared<SharedContext*>()->webApi.GetPlayerData(_userId.value(), _playMode);
                    _playingPlayerDataStatus = _LoadStatus::LOADING;
                }
                else if (_loggedInPlayerData)
                {
                    if (_loggedInPlayerData->userId != "-1")
                    {
                        _playingPlayerData = _loggedInPlayerData;
                        _playingPlayerDataStatus = _LoadStatus::LOADED;
                        afterPlayingPlayerLoad();
                    }
                    else if (!_playingPlayerUnresolvableWarningShown)
                    {
                        StandardNotificationTemplate notif;
                        notif.type = StandardNotificationTemplate::NotificationType::WARNING;
                        notif.title = L"No player to show";
                        notif.text = L"User not logged in or no other user specified in leaderboard parameters. The leaderboard will not be shown";
                        notif.showDuration = Duration(6, SECONDS);
                        _scene->GetApp()->Shared<SharedContext*>()->notificationService.ShowNotification(notif.ToNotificationInfo());
                        _playingPlayerUnresolvableWarningShown = true;
                    }
                }
            }
        }
        else if (_playingPlayerDataStatus == _LoadStatus::LOADING)
        {
            _otherPlayerDataSubscription->HandlePendingEvents([&](const webapi::resp::WebResponse<webapi::resp::PlayerData>& response) {
                if (response.status != 200 || !response.content.has_value())
                {
                    // Try again after delay
                    ExecuteSynchronously([=]() {
                        _playingPlayerDataStatus = _LoadStatus::NOT_LOADED;
                        _playingPlayerData = std::nullopt;
                    }, Duration(5, SECONDS));
                    return;
                }

                _PlayerData data{};
                data.userId = response.content->userId;
                data.username = response.content->username;
                data.mode = response.content->mode;
                data.pp = response.content->pp;
                data.ppSumOfBestScores = 0.0f;
                for (auto& score : response.content->scores)
                {
                    data.scores.push_back({ 0, score.pp, score.ppWeighted, score.mapId });
                    data.ppSumOfBestScores += score.ppWeighted;
                }
                data.scoreCount = -1;

                _playingPlayerData = data;
                _playingPlayerDataStatus = _LoadStatus::LOADED;
                afterPlayingPlayerLoad();
            });
        }
        else if (_playingPlayerDataStatus == _LoadStatus::LOADED)
        {
            if (_playing && _playMode != _playingPlayerData->mode)
            {
                _playingPlayerData = std::nullopt;
                _playingPlayerDataStatus = _LoadStatus::NOT_LOADED;
            }
        }

        if (_playing && _playingPlayerData)
        {
            if (_leaderboardStatus == _LoadStatus::NOT_LOADED)
            {
                _initialLeaderboardSubscription = _scene->GetApp()->Shared<SharedContext*>()->webApi.GetInitialLeaderboard(_playingPlayerData->pp, _playingPlayerData->mode, _countryCode);
                _leaderboardStatus = _LoadStatus::LOADING;
            }
            else if (_leaderboardStatus == _LoadStatus::LOADING)
            {
                _initialLeaderboardSubscription->HandlePendingEvents([=](const webapi::resp::WebResponse<std::vector<webapi::resp::LeaderboardPage>>& response) {
                    if (response.status != 200 || !response.content.has_value())
                    {
                        // Try again after delay
                        ExecuteSynchronously([=]() {
                            _leaderboardStatus = _LoadStatus::NOT_LOADED;
                        }, Duration(5, SECONDS));
                        return;
                    }

                    _leaderboard.clear();
                    for (auto& responsePage : response.content.value())
                    {
                        _LeaderboardPage page{};
                        page.pageNumber = responsePage.pageNumber;
                        for (int i = 0; i < responsePage.users.size(); i++)
                        {
                            _User user{};
                            user.globalRank = 50 * (page.pageNumber - 1) + 1 + i;
                            user.indexRank = user.globalRank;
                            user.pp = responsePage.users[i].pp;
                            user.username = responsePage.users[i].username;
                            page.users.push_back(user);
                        }
                        _leaderboard.push_back(page);
                    }

                    _playingPlayerInitialRank = std::nullopt;
                    for (int i = 0; i < _leaderboard.size(); i++)
                    {
                        bool breakOutside = false;
                        for (int j = 0; j < _leaderboard[i].users.size(); j++)
                        {
                            if (_leaderboard[i].users[j].username == _playingPlayerData->username)
                            {
                                _playingPlayerInitialRank = _leaderboard[i].users[j].indexRank;
                                breakOutside = true;
                                break;
                            }
                        }
                        if (breakOutside)
                            break;
                    }

                    if (_playingPlayerInitialRank)
                        _currentRank = _playingPlayerInitialRank.value();
                    else if (!_leaderboard.empty() && !_leaderboard.back().users.empty())
                        _currentRank = (int)_leaderboard.back().users.back().indexRank;
                    else
                    {
                        _currentRank = 1;

                        StandardNotificationTemplate notif;
                        notif.type = StandardNotificationTemplate::NotificationType::WARNING;
                        notif.title = L"No leaderboard to show";
                        notif.text = L"Either the leaderboard is empty, or the load failed";
                        notif.showDuration = Duration(6, SECONDS);
                        _scene->GetApp()->Shared<SharedContext*>()->notificationService.ShowNotification(notif.ToNotificationInfo());
                    }

                    _leaderboardStatus = _LoadStatus::LOADED;
                });
            }
        }

        if (_playing && _playingPlayerData)
        {
            if (!_extraPlayDataLoaded)
            {
                for (int i = 0; i < _playingPlayerData->scores.size(); i++)
                {
                    if (_playingPlayerData->scores[i].mapId == state.beatmap.id)
                    {
                        _currentMapIndexInPlayerTopScores = i;
                        break;
                    }
                }
                if (!_currentMapIndexInPlayerTopScores)
                    _currentMapIndexInPlayerTopScores = -1;
                
                _userLeaderboardItem->SetUsername(string_to_wstring(_playingPlayerData->username));

                _extraPlayDataLoaded = true;
            }
        }

        if (_playing && _playingPlayerData && _leaderboardStatus == _LoadStatus::LOADED && !_leaderboard.empty())
        {
            _ShowLeaderboard();

            _UpdateCurrentPlayData(state);
            _UpdateLeaderboardItems();

            int highestLoadedIndexRank = (int)(_leaderboard.front().pageNumber - 1) * 50 + 1;
            if (!_loadingLeaderboardPage && _leaderboard.front().pageNumber > 1 && highestLoadedIndexRank + 50 > _currentRank)
            {
                _leaderboardPageSubscription = _scene->GetApp()->Shared<SharedContext*>()->webApi.GetLeaderboardPage((int)_leaderboard.front().pageNumber - 1, _playingPlayerData->mode, _countryCode);
                _loadingLeaderboardPage = true;
            }
            if (_loadingLeaderboardPage)
            {
                _leaderboardPageSubscription->HandlePendingEvents([=](const webapi::resp::WebResponse<webapi::resp::LeaderboardPage>& response) {
                    if (response.status != 200 || !response.content.has_value())
                    {
                        // Try again after delay
                        ExecuteSynchronously([=]() {
                            _loadingLeaderboardPage = false;
                        }, Duration(5, SECONDS));
                        return;
                    }

                    _LeaderboardPage page{};
                    page.pageNumber = response.content->pageNumber;
                    for (int i = 0; i < response.content->users.size(); i++)
                    {
                        _User user{};
                        user.globalRank = 50 * (page.pageNumber - 1) + 1 + i;
                        user.indexRank = user.globalRank;
                        user.pp = response.content->users[i].pp;
                        user.username = response.content->users[i].username;
                        page.users.push_back(user);
                    }
                    _leaderboard.insert(_leaderboard.begin(), page);
                    _loadingLeaderboardPage = false;
                });
            }
        }
        else
        {
            _HideLeaderboard();
        }

        bool inResultsScreen = state.state.number == 7;
        if (inResultsScreen && _previousResultId == 0 && state.resultsScreen.scoreId != 0)
        {
            bool mapIsRanked = state.beatmap.status.number == 4;
            // Player score count (which is used to calculate bonus PP) only increases when submitting ranked scores and is capped at 1000
            if (mapIsRanked && _loggedInPlayerData)
            {
                _EvaluateNewScore(state.beatmap.id, std::to_string(state.profile.id), state.resultsScreen.scoreId);
            }
        }

        _previousResultId = state.resultsScreen.scoreId;
        _previousState = state.state.number;
    }
    else
    {
        _HideLeaderboard();

        // Clear logged in player data, so that it is loaded again once data becomes available
        _loggedInPlayerData = std::nullopt;
        _loggedInPlayerDataStatus = _LoadStatus::NOT_LOADED;
    }

    if (_movingIn || _movingOut)
    {
        float x = (float)(ztime::Main() - _moveXStart).GetDuration() / _moveXDuration.GetDuration();
        if (x >= 1.0f)
        {
            _mainPanel->position.Assign(X(_movingIn ? 0 : -_mainPanel->size_->width));
            if (_movingOut)
            {
                _mainPanel->visible = false;
                _leaderboardPanel->ClearItems();
                _leaderboardItems.clear();
                _movingOut = false;
            }
            else
            {
                _movingIn = false;
            }
        }
        else
        {
            int startPosition = _movingIn ? -_mainPanel->size_->width : 0;
            int targetPosition = _movingIn ? 0 : -_mainPanel->size_->width;
            int currentPosition = zanim::Interpolate(startPosition, targetPosition, _movingIn ? zanim::EaseOutPow(x, 2.0f) : zanim::EaseInPow(x, 2.0f));
            _mainPanel->position.Assign(X(currentPosition));
        }
        // TODO: test what happens if this is removed
        ForceLayoutUpdate();
    }
}

float CalculateBonusPPForScoreCount(int scoreCount)
{
    return 416.6667f * (1 - std::powf(0.995f, (float)std::min(scoreCount + 1, 1000)));
}

void zcom::RTLeaderboardOverlayComponent::_UpdateCurrentPlayData(const osu::GameState& state)
{
    float currentPP = state.play.pp.current;
    int currentPPRank = (int)_playingPlayerData->scores.size();
    for (int i = currentPPRank - 1; i >= 0; i--)
    {
        if (_playingPlayerData->scores[i].pp > currentPP)
            break;
        currentPPRank = i;
    }

    int mapIndexInScores = _currentMapIndexInPlayerTopScores.value();

    int newRank = _currentRank;
    float profilePP = _playingPlayerData->pp;

    // If score count is unknown, calculate bonus pp by subtracting the pp sum of best scores from total profile pp
    // For players with LESS than 200 best scores, this gives an exact value
    // For players with MORE than 200 best scores, we get a very close to exact value, since those scores account for well above 99% of all score pp
    //  and the resulting difference in calculating profile pp is ~0.01pp for a player with 9000pp
    //  (players with more pp probably have max bonus pp which gets detected after a SINGLE submitted score and even this small difference disappears)
    static float MAX_BONUS_PP = CalculateBonusPPForScoreCount(1000);
    float maxBonusPPForUser = std::min(profilePP - _playingPlayerData->ppSumOfBestScores, MAX_BONUS_PP);
    // Estimate a score count from potentially approximate bonus pp
    int estimatedScoreCount = (int)std::roundf(std::logf(1.0f - maxBonusPPForUser / 416.6667f) / std::logf(0.995f));
    float bonusPP = CalculateBonusPPForScoreCount(estimatedScoreCount);
    float bonusPPAfterPlay = CalculateBonusPPForScoreCount(estimatedScoreCount + 1);
    if (_playingPlayerData->scoreCount != -1)
    {
        float exactBonusPP = CalculateBonusPPForScoreCount(_playingPlayerData->scoreCount);
        // Defensive: if some calculation gives a score count value above what's possible, just use the original - still quite accurate - estimation
        if (exactBonusPP < maxBonusPPForUser)
        {
            bonusPP = exactBonusPP;
            bool mapIsRanked = state.beatmap.status.number == 4;
            bonusPPAfterPlay = mapIsRanked ? CalculateBonusPPForScoreCount(_playingPlayerData->scoreCount + 1) : exactBonusPP;
        }
    }
    float bonusPPIncrease = bonusPPAfterPlay - bonusPP;

    float newTotalPP = profilePP;
    if (currentPPRank < _playingPlayerData->scores.size())
    {
        if (mapIndexInScores == -1)
        {
            float ppSumAboveScore = 0.0f;
            for (int i = 0; i < currentPPRank; i++)
                ppSumAboveScore += _playingPlayerData->scores[i].ppWeighted;
            newTotalPP = (profilePP - bonusPP - ppSumAboveScore) * 0.95f + bonusPPAfterPlay + ppSumAboveScore + currentPP * std::powf(0.95f, (float)currentPPRank);
        }
        else if (currentPPRank <= mapIndexInScores)
        {
            float ppSumBetweenOldAndNewScore = 0.0f;
            for (int i = currentPPRank; i < mapIndexInScores; i++)
                ppSumBetweenOldAndNewScore += _playingPlayerData->scores[i].ppWeighted;
            newTotalPP = profilePP - ppSumBetweenOldAndNewScore * 0.05f + currentPP * std::powf(0.95f, (float)currentPPRank) - _playingPlayerData->scores[mapIndexInScores].ppWeighted + bonusPPIncrease;
        }
        else
        {
            newTotalPP = profilePP + bonusPPIncrease;
        }
    }
    else
    {
        newTotalPP = profilePP + bonusPPIncrease;
    }
    for (int i = 0; i < _leaderboard.size(); i++)
    {
        bool breakOutside = false;
        for (int j = 0; j < _leaderboard[i].users.size(); j++)
        {
            if (_leaderboard[i].users[j].pp <= newTotalPP)
            {
                newRank = (int)_leaderboard[i].users[j].indexRank;
                breakOutside = true;
                break;
            }
        }
        if (breakOutside)
            break;
    }

    _previousRank = _currentRank;
    _currentRank = newRank;
    _currentTotalPP = newTotalPP;
}

void zcom::RTLeaderboardOverlayComponent::_UpdateLeaderboardItems()
{
    int highestLoadedIndexRank = (int)(_leaderboard.front().pageNumber - 1) * 50 + 1;
    int lowestLoadedIndexRank = (int)(_leaderboard.back().pageNumber - 1) * 50 + _leaderboard.back().users.size();

    int userPosition = (_currentRank - highestLoadedIndexRank) + 1;
    if (userPosition > _visibleItemCount)
        userPosition = _visibleItemCount;

    // Find range of ranks that must be visible during next scroll
    int highestVisibleIndexRank = _previousRank - (_visibleItemCount - 1);
    int lowestVisibleIndexRank = _previousRank - 1;
    if (_previousRank < highestLoadedIndexRank + (_visibleItemCount - 1))
    {
        highestVisibleIndexRank = highestLoadedIndexRank;
        lowestVisibleIndexRank = highestLoadedIndexRank + (_visibleItemCount - 2);
    }
    int newHighestVisibleIndexRank = _currentRank - (_visibleItemCount - 1);
    int newLowestVisibleIndexRank = _currentRank - 1;
    if (_currentRank < highestLoadedIndexRank + (_visibleItemCount - 1))
    {
        newHighestVisibleIndexRank = highestLoadedIndexRank;
        newLowestVisibleIndexRank = highestLoadedIndexRank + (_visibleItemCount - 2);
    }
    if (newHighestVisibleIndexRank < highestVisibleIndexRank)
        highestVisibleIndexRank = newHighestVisibleIndexRank;
    if (newLowestVisibleIndexRank > lowestVisibleIndexRank)
        lowestVisibleIndexRank = newLowestVisibleIndexRank;

    // Find range of ranks that must be visible at current rank
    int highestCurrentlyVisibleIndexRank = _currentRank - (_visibleItemCount - 1);
    int lowestCurrentlyVisibleIndexRank = _currentRank - 1;
    if (_currentRank < highestLoadedIndexRank + (_visibleItemCount - 1))
    {
        highestCurrentlyVisibleIndexRank = highestLoadedIndexRank;
        lowestCurrentlyVisibleIndexRank = highestLoadedIndexRank + (_visibleItemCount - 2);
    }

    bool moveItems = _currentRank != _previousRank;
    if (_visibleItemCountChanged || _scaleChanged)
    {
        moveItems = true;
        _visibleItemCountChanged = false;
        _scaleChanged = false;
    }

    _leaderboardPanel->DeferLayoutUpdates();

    _userLeaderboardItem->SetPP(_currentTotalPP, _currentTotalPP - _playingPlayerData->pp);
    if (_leaderboardItems.empty())
    {
        // Load initial items
        _leaderboardItems.clear();
        _currentGlobalOffset = (highestCurrentlyVisibleIndexRank - 1) * (_itemSize->height + _itemSpacing);
        for (int indexRank = highestVisibleIndexRank; indexRank <= lowestVisibleIndexRank && indexRank <= lowestLoadedIndexRank; indexRank++)
        {
            auto newItem = _CreateLeaderboardItemForRank(indexRank);
            _leaderboardPanel->AddItem(newItem.get());
            _leaderboardItems.push_back(std::move(newItem));
        }
        _userLeaderboardItem->Move(_itemSpacing + (userPosition - 1) * (_itemSize->height + _itemSpacing), Duration(0));
        _leaderboardPanel->AddItem(_userLeaderboardItem.get());
        _ShowLeaderboard();
    }
    else if (!_leaderboardItems.empty())
    {
        int topRank = (int)_leaderboardItems.front()->GetRank();
        int itemsAddedAbove = 0;
        for (int indexRank = highestVisibleIndexRank; indexRank <= lowestVisibleIndexRank && indexRank <= lowestLoadedIndexRank; indexRank++)
        {
            if (topRank > indexRank)
            {
                auto newItem = _CreateLeaderboardItemForRank(indexRank);
                _leaderboardPanel->AddItem(newItem.get());
                _leaderboardItems.insert(_leaderboardItems.begin() + itemsAddedAbove++, std::move(newItem));
            }
            else if (_leaderboardItems.back()->GetRank() < indexRank)
            {
                auto newItem = _CreateLeaderboardItemForRank(indexRank);
                // All new items below start shifted down
                newItem->Move(_itemSize->height + _itemSpacing, Duration(0));
                newItem->SetRankOffset(1);
                _leaderboardPanel->AddItem(newItem.get());
                _leaderboardItems.push_back(std::move(newItem));
            }
        }
    }
    // Update item state
    for (auto& item : _leaderboardItems)
    {
        if (item->GetRank() < highestCurrentlyVisibleIndexRank || item->GetRank() > lowestCurrentlyVisibleIndexRank)
            item->FadeOut();
        else if (item->Fading())
            item->FadeIn();
    }
    // Remove destroyed items
    for (int i = 0; i < _leaderboardItems.size(); i++)
    {
        if (_leaderboardItems[i]->Destroy())
        {
            _leaderboardPanel->RemoveItem(_leaderboardItems[i].get());
            _leaderboardItems.erase(_leaderboardItems.begin() + i);
            i--;
        }
    }
    // Make closest items visible
    int closestItemIndex = -1;
    int closestItemDistance = std::numeric_limits<int>::max();
    for (int i = 0; i < _leaderboardItems.size(); i++)
    {
        int itemPosition = _leaderboardItems[i]->position->y;
        int playerPosition = _userLeaderboardItem->position->y;
        int distance = std::abs(itemPosition - playerPosition);
        if (distance < closestItemDistance)
        {
            closestItemDistance = distance;
            closestItemIndex = i;
        }
    }
    if (closestItemIndex != -1)
    {
        int itemsLeftToUpdate = 5;
        int currentOffset = 0;
        while (itemsLeftToUpdate > 0)
        {
            int leftIndex = closestItemIndex - currentOffset;
            if (leftIndex >= 0 && !_leaderboardItems[leftIndex]->visible)
            {
                _leaderboardItems[leftIndex]->visible = true;
                itemsLeftToUpdate--;
            }

            int rightIndex = closestItemIndex + currentOffset;
            if (rightIndex < _leaderboardItems.size() && !_leaderboardItems[rightIndex]->visible)
            {
                _leaderboardItems[rightIndex]->visible = true;
                itemsLeftToUpdate--;
            }

            currentOffset++;

            if (leftIndex <= 0 && rightIndex >= _leaderboardItems.size() - 1)
                break;
        }
    }

    // Update global offset
    if (_movingItems)
    {
        float x = (float)(ztime::Main() - _moveStart).GetDuration() / _moveDuration.GetDuration();
        if (x >= 1.0f)
        {
            _currentGlobalOffset = _targetGlobalOffset;
            _movingItems = false;
        }
        else
        {
            _currentGlobalOffset = zanim::Interpolate(_startGlobalOffset, _targetGlobalOffset, zanim::EaseOutPow(x, 2.0f));
        }
    }
    if (moveItems)
    {
        int targetGlobalOffset = (highestCurrentlyVisibleIndexRank - 1) * (_itemSize->height + _itemSpacing);
        _movingItems = true;
        _startGlobalOffset = _currentGlobalOffset;
        _targetGlobalOffset = targetGlobalOffset;
        _moveStart = ztime::Main();
    }

    bool movingDown = _targetGlobalOffset > _startGlobalOffset;
    if (_userLeaderboardItem->Moving())
        movingDown = !_userLeaderboardItem->MovingUp();

    // Calculate item positions
    for (auto& item : _leaderboardItems)
    {
        int absoluteItemOffset = int(_itemSpacing + (item->GetRank() - 1) * (_itemSize->height + _itemSpacing));
        int baseVisualPosition = absoluteItemOffset - _currentGlobalOffset;

        // Offset user items below player
        if (movingDown
            ? baseVisualPosition > _userLeaderboardItem->Offset()
            : baseVisualPosition > _userLeaderboardItem->Offset() - (_itemSize->height + _itemSpacing))
        {
            item->Move(_itemSize->height + _itemSpacing, Duration(150, MILLISECONDS));
            item->SetRankOffset(1);
        }
        else
        {
            item->Move(0, Duration(150, MILLISECONDS));
            item->SetRankOffset(0);
        }

        item->position.Assign(Y(baseVisualPosition + item->Offset()));
    }
    _userLeaderboardItem->Move(_itemSpacing + (userPosition - 1) * (_itemSize->height + _itemSpacing), Duration(500, MILLISECONDS));
    _userLeaderboardItem->position.Assign(Y(_userLeaderboardItem->Offset()));

    int userGlobalOffset = _currentGlobalOffset + _userLeaderboardItem->Offset() - _itemSpacing;
    int rankFromOffset = movingDown
        ? int(std::ceilf(userGlobalOffset / float(_itemSize->height + _itemSpacing)) + 1)
        : int(std::floorf(userGlobalOffset / float(_itemSize->height + _itemSpacing)) + 1);
    _userLeaderboardItem->SetRank(rankFromOffset);

    if (moveItems)
        _userLeaderboardItem->PopOut();

    _leaderboardPanel->ResumeLayoutUpdates(true, true);
}

void zcom::RTLeaderboardOverlayComponent::_EvaluateNewScore(std::string mapId, std::string userId, int64_t scoreId)
{
    _newScoreLoadStatus = _LoadStatus::LOADING;

    EventEmitter<void, std::optional<_PlayerData>> evaluatedScoreEventEmitter(EventEmitterThreadMode::MULTITHREADED);
    _evaluatedScoreSubscription = evaluatedScoreEventEmitter->SubscribeAsync([=](std::optional<_PlayerData> playerData) {
        ExecuteSynchronously([=]() {
            if (playerData && playerData->userId == userId)
            {
                _loggedInPlayerData->pp = playerData->pp;
                _loggedInPlayerData->scoreCount = playerData->scoreCount;
                _loggedInPlayerData->scores = playerData->scores;
                _loggedInPlayerData->ppSumOfBestScores = playerData->ppSumOfBestScores;
            }
            _newScoreLoadStatus = _LoadStatus::LOADED;
        });
    });

    webapi::WebApi webApi(_scene->GetApp()->config.GetConfigValue(webapi::WebApiConfig::API_URL));

    std::thread([evaluatedScoreEventEmitter, webApi, mapId, userId, scoreId, currentPlayerData = _loggedInPlayerData.value()]() {
        std::optional<_PlayerData> newPlayerData = std::nullopt;
        bool scoreFound = false;
        float scorePP = 0.0f;
        bool scoreIsBest = false;
        for (int i = 0; i < 5; i++)
        {
            auto scoreSub = webApi.GetBeatmapUserScore(mapId, userId, currentPlayerData.mode, scoreId);
            SimpleTimer scoreTimeoutTimer;
            while (!scoreSub->EventsPending() && scoreTimeoutTimer.SecondsElapsed() < 10)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (!(scoreTimeoutTimer.SecondsElapsed() < 10))
                continue;

            scoreSub->HandlePendingEvents([&](const webapi::resp::WebResponse<webapi::resp::BeatmapUserScore>& response) {
                if (response.status != httplib::OK_200)
                    return;

                scoreFound = true;
                if (response.content.has_value())
                {
                    scorePP = response.content->pp;
                    scoreIsBest = response.content->isBest;
                }
            });
            if (!scoreFound)
            {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                continue;
            }

            auto userSub = webApi.GetPlayerData(userId, currentPlayerData.mode);
            SimpleTimer userTimeoutTimer;
            while (!userSub->EventsPending() && userTimeoutTimer.SecondsElapsed() < 10)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (!(userTimeoutTimer.SecondsElapsed() < 10))
                continue;

            userSub->HandlePendingEvents([&](const webapi::resp::WebResponse<webapi::resp::PlayerData>& response) {
                if (response.status != 200 || !response.content.has_value())
                    return;

                _PlayerData data{};
                data.userId = response.content->userId;
                data.username = response.content->username;
                data.pp = response.content->pp;
                data.ppSumOfBestScores = 0.0f;
                for (auto& score : response.content->scores)
                {
                    data.scores.push_back({ 0, score.pp, score.ppWeighted, score.mapId });
                    data.ppSumOfBestScores += score.ppWeighted;
                }
                newPlayerData = data;
            });

            break;
        }

        if (scoreFound && newPlayerData)
        {
            newPlayerData->scoreCount = currentPlayerData.scoreCount;

            // Score is inside top plays
            if (scoreIsBest && !currentPlayerData.scores.empty() && currentPlayerData.scores.back().pp < scorePP)
            {
                bool isNewScore = streams::From(currentPlayerData.scores)
                    .NoneMatch([=](const _PlayerData::_Score& score) { return score.mapId == mapId; });
                if (isNewScore)
                {
                    int newScoreRank = (int) currentPlayerData.scores.size();
                    float ppSumAbovePlay = 0.0f;
                    for (int i = 0; i < currentPlayerData.scores.size(); i++)
                    {
                        if (currentPlayerData.scores[i].pp < scorePP)
                        {
                            newScoreRank = i;
                            break;
                        }
                        ppSumAbovePlay += currentPlayerData.scores[i].ppWeighted;
                    }

                    float p0 = currentPlayerData.pp;
                    float p1 = newPlayerData->pp;
                    float pa = ppSumAbovePlay;
                    float c = scorePP;
                    float n = (float)newScoreRank;
                    float Bmax = 416.6667f;

                    float maxedBonusPPScoreCountEstimate = std::logf(1.0f - (0.95f / (Bmax * 0.05f)) * ((p1 - pa - c * std::powf(0.95f, n)) / 0.95f - p0 + pa)) / std::logf(0.995f);
                    if ((int)std::roundf(maxedBonusPPScoreCountEstimate) == 1000)
                    {
                        // Max bonus pp reached
                        newPlayerData->scoreCount = 1000;
                    }
                    else
                    {
                        float part1 = (p1 - pa - c * std::powf(0.95f, n)) / 0.95f - p0 + pa + Bmax - Bmax / 0.95f;
                        float part2 = Bmax - (Bmax / 0.95f) * 0.995f;
                        float nonMaxedBonusPPScoreCountEstimate = std::logf(part1 / part2) / std::logf(0.995f);

                        // Allow score count to fluctuate up and down to account for any errors in ppGain value (if it's erroneously 0, the estimated score count is immediatelly 1000)
                        newPlayerData->scoreCount = (int)std::roundf(nonMaxedBonusPPScoreCountEstimate);
                    }
                }
                else
                {
                    if (newPlayerData->scoreCount != -1)
                        newPlayerData->scoreCount++;
                }
            }
            // Score is outside top plays
            else
            {
                float ppGain = newPlayerData->pp - currentPlayerData.pp;
                if (ppGain == 0)
                    newPlayerData->scoreCount = 1000;
            }

            if (newPlayerData->scoreCount < 1)
                newPlayerData->scoreCount = 1;
            else if (newPlayerData->scoreCount > 1000)
                newPlayerData->scoreCount = 1000;

            evaluatedScoreEventEmitter->InvokeAll(newPlayerData);
        }
        else
        {
            evaluatedScoreEventEmitter->InvokeAll(std::nullopt);
        }
    }).detach();
}

void zcom::RTLeaderboardOverlayComponent::_ShowLeaderboard()
{
    if (_movingIn || !_leaderboardHidden)
        return;

    _leaderboardHidden = false;
    _movingIn = true;
    _movingOut = false;
    _moveXStart = ztime::Main();
    _mainPanel->visible = true;
}

void zcom::RTLeaderboardOverlayComponent::_HideLeaderboard()
{
    if (_movingOut || _leaderboardHidden)
        return;

    _leaderboardHidden = true;
    _movingIn = false;
    _movingOut = true;
    _moveXStart = ztime::Main();
}

zcom::RTLeaderboardOverlayComponent::_User* zcom::RTLeaderboardOverlayComponent::_GetUserByIndexRank(int indexRank)
{
    if (_leaderboard.empty())
        return nullptr;
    int indexPage = (indexRank - 1) / 50 + 1;
    if (_leaderboard.back().pageNumber < indexPage)
        return nullptr;
    if (_leaderboard.front().pageNumber > indexPage)
        return nullptr;
    int startPage = (int)_leaderboard.front().pageNumber;
    int userIndexInPage = (indexRank - 1) % 50;

    return &_leaderboard[indexPage - startPage].users[userIndexInPage];
}

std::unique_ptr<zcom::LeaderboardItem> zcom::RTLeaderboardOverlayComponent::_CreateLeaderboardItemForRank(int indexRank)
{
    auto item = Create<LeaderboardItem>(&SCALE);
    item->size.ComputedFrom([](Size size, bool visible) { return visible ? size : Size{ 0, 0 }; }, _itemSize, item->visible);
    item->backgroundColor.ComputedFrom([](Color color) { return color; }, _nonPlayerBackgroundColor);
    item->GetUsernameLabel()->fontColor.ComputedFrom([](Color color) { return color; }, _usernameTextColor);
    item->GetPPLabel()->fontColor.ComputedFrom([](Color color) { return color; }, _ppTextColor);
    item->GetRankLabel()->fontColor.ComputedFrom([](Color color) { return color; }, _rankTextColor);

    // Skip the playing player itself to avoid seeing multiple copies of the same player (only relevant for like the top 3 players)
    bool skip = _playingPlayerInitialRank.has_value() && indexRank >= _playingPlayerInitialRank.value();

    _User* user = _GetUserByIndexRank(skip ? indexRank + 1 : indexRank);
    if (!user)
    {
        item->SetUsername(L"-");
        item->SetRank(-1);
        item->SetPP(0);
    }
    else
    {
        item->SetUsername(string_to_wstring(user->username));
        item->SetRank(skip ? user->indexRank - 1 : user->indexRank);
        item->SetPP(user->pp);
    }
    // All items start invisible and are made visible in limited counts per frame, to avoid lag spikes when lots of movement is happening in the leaderboard
    item->visible = false;
    return item;
}