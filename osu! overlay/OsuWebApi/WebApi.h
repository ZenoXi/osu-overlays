#pragma once

#include "WebApiConfig.h"
#include "Helper/EventEmitter.h"
#include "Helper/StringHelper.h"
#include "Helper/Time.h"
#include "Shared/Util/Http.h"
#include "Shared/Util/JsonParser.h"
#include "Response/WebResponse.h"
#include "Response/LeaderboardPage.h"
#include "Response/PlayerData.h"
#include "Response/BeatmapUserScore.h"

#include <string>
#include <thread>
#include <mutex>
#include <optional>

namespace webapi
{
    class WebApi
    {
        std::wstring _url;

    public:
        WebApi(std::wstring url = WebApiConfig::API_URL.defaultValue)
        {
            _url = url;
        }

        void SetUrl(const std::wstring& url)
        {
            _url = url;
        }

        std::unique_ptr<AsyncEventSubscription<void, resp::WebResponse<resp::PlayerData>>> GetPlayerData(const std::string& userId, const std::string& mode) const
        {
            auto resultEmitter = EventEmitter<void, resp::WebResponse<resp::PlayerData>>(EventEmitterThreadMode::MULTITHREADED);
            auto subscription = resultEmitter->SubscribeAsync();

            std::thread([resultEmitter, userId, mode, url = _url]() {
                httplib::Client cli(wstring_to_string(url));
                httplib::Headers headers = {
                    { "Content-Type", "application/json" },
                    { "Accept", "application/json" }
                };
                auto resp = cli.Get("/api/v1/playerData?userId=" + userId + "&mode=" + mode, headers);
                if (!resp || resp->status != httplib::OK_200)
                {
                    resultEmitter->InvokeAll({ resp ? resp->status : -1, "Failed to retrieve player data", std::nullopt });
                    return;
                }

                try
                {
                    zjson::JsonParser parser(resp->body);
                    resp::PlayerData data{};
                    data.userId = parser.GetString(".userId");
                    data.username = parser.GetString(".username");
                    data.mode = mode;
                    data.pp = parser.GetFloat(".pp");
                    size_t scoreCount = parser.GetArraySize(".scores");
                    for (int i = 0; i < scoreCount; i++)
                    {
                        resp::PlayerData::Score score{};
                        score.pp = parser.GetFloat(".scores[" + std::to_string(i) + "].pp");
                        score.ppWeighted = parser.GetFloat(".scores[" + std::to_string(i) + "].ppWeighted");
                        score.mapId = parser.GetString(".scores[" + std::to_string(i) + "].mapId");
                        data.scores.push_back(score);
                    }
                    resultEmitter->InvokeAll({ httplib::OK_200, "", data });
                }
                catch (zjson::ParseException e)
                {
                    resultEmitter->InvokeAll({ 0, "Player data parsing failed: " + std::string(e.what()), std::nullopt });
                }
            }).detach();

            return subscription;
        }

        std::unique_ptr<AsyncEventSubscription<void, resp::WebResponse<std::vector<resp::LeaderboardPage>>>> GetInitialLeaderboard(float pp, const std::string& mode, std::optional<std::string> countryCode = std::nullopt) const
        {
            auto resultEmitter = EventEmitter<void, resp::WebResponse<std::vector<resp::LeaderboardPage>>>(EventEmitterThreadMode::MULTITHREADED);
            auto subscription = resultEmitter->SubscribeAsync();

            std::thread([resultEmitter, pp, mode, countryCode, url = _url]() {
                httplib::Client cli(wstring_to_string(url));
                httplib::Headers headers = {
                    { "Content-Type", "application/json" },
                    { "Accept", "application/json" }
                };
                auto resp = cli.Get("/api/v1/initialLeaderboard?pp=" + std::to_string(pp) + "&mode=" + mode + (countryCode ? "&country=" + countryCode.value() : ""), headers);
                if (!resp || resp->status != httplib::OK_200)
                {
                    resultEmitter->InvokeAll({ resp ? resp->status : -1, "Failed to retrieve initial leaderboard", std::nullopt });
                    return;
                }

                try
                {
                    zjson::JsonParser parser(resp->body);

                    std::vector<resp::LeaderboardPage> pages;
                    size_t pageCount = parser.GetArraySize("");
                    for (int i = 0; i < pageCount; i++)
                    {
                        resp::LeaderboardPage page{};
                        page.pageNumber = parser.GetInt("[" + std::to_string(i) + "].pageNumber");
                        size_t pageSize = parser.GetArraySize("[" + std::to_string(i) + "].users");
                        for (int j = 0; j < pageSize; j++)
                        {
                            resp::LeaderboardPage::User user{};
                            //user.globalRank = 50 * (page.pageNumber - 1) + 1 + j;
                            //user.indexRank = user.globalRank;
                            user.username = parser.GetString("[" + std::to_string(i) + "].users[" + std::to_string(j) + "].username");
                            user.pp = parser.GetFloat("[" + std::to_string(i) + "].users[" + std::to_string(j) + "].pp");
                            page.users.push_back(std::move(user));
                        }
                        pages.push_back(page);
                    }
                    resultEmitter->InvokeAll({ httplib::OK_200, "", pages });
                }
                catch (zjson::ParseException e)
                {
                    resultEmitter->InvokeAll({ 0, "Initial leaderboard parsing failed: " + std::string(e.what()), std::nullopt });
                }
            }).detach();

            return subscription;
        }

        std::unique_ptr<AsyncEventSubscription<void, resp::WebResponse<resp::LeaderboardPage>>> GetLeaderboardPage(int pageNumber, const std::string& mode, std::optional<std::string> countryCode = std::nullopt) const
        {
            auto resultEmitter = EventEmitter<void, resp::WebResponse<resp::LeaderboardPage>>(EventEmitterThreadMode::MULTITHREADED);
            auto subscription = resultEmitter->SubscribeAsync();

            std::thread([resultEmitter, pageNumber, mode, countryCode, url = _url]() {
                httplib::Client cli(wstring_to_string(url));
                httplib::Headers headers = {
                    { "Content-Type", "application/json" },
                    { "Accept", "application/json" }
                };
                auto resp = cli.Get("/api/v1/leaderboardPage?pageNumber=" + std::to_string(pageNumber) + "&mode=" + mode + (countryCode ? "&country=" + countryCode.value() : ""), headers);
                if (!resp || resp->status != httplib::OK_200)
                {
                    resultEmitter->InvokeAll({ resp ? resp->status : -1, "Failed to retrieve leaderboard page", std::nullopt });
                    return;
                }

                try
                {
                    zjson::JsonParser parser(resp->body);

                    resp::LeaderboardPage page{};
                    page.pageNumber = parser.GetInt(".pageNumber");
                    size_t pageSize = parser.GetArraySize(".users");
                    for (int i = 0; i < pageSize; i++)
                    {
                        resp::LeaderboardPage::User user{};
                        user.username = parser.GetString(".users[" + std::to_string(i) + "].username");
                        user.pp = parser.GetFloat(".users[" + std::to_string(i) + "].pp");
                        page.users.push_back(std::move(user));
                    }
                    resultEmitter->InvokeAll({ httplib::OK_200, "", page });
                }
                catch (zjson::ParseException e)
                {
                    resultEmitter->InvokeAll({ 0, "Leaderboard page parsing failed: " + std::string(e.what()), std::nullopt });
                }
            }).detach();

            return subscription;
        }

        std::unique_ptr<AsyncEventSubscription<void, resp::WebResponse<resp::BeatmapUserScore>>> GetBeatmapUserScore(std::string beatmapId, std::string userId, const std::string& mode, std::optional<int64_t> scoreId = std::nullopt) const
        {
            auto resultEmitter = EventEmitter<void, resp::WebResponse<resp::BeatmapUserScore>>(EventEmitterThreadMode::MULTITHREADED);
            auto subscription = resultEmitter->SubscribeAsync();

            std::thread([resultEmitter, beatmapId, userId, mode, scoreId, url = _url]() {
                httplib::Client cli(wstring_to_string(url));
                httplib::Headers headers = {
                    { "Content-Type", "application/json" },
                    { "Accept", "application/json" }
                };
                std::string url = "/api/v1/beatmapUserScore?beatmapId=" + beatmapId + "&userId=" + userId + "&mode=" + mode;
                if (scoreId)
                    url += "&scoreId=" + std::to_string(scoreId.value());
                auto resp = cli.Get(url, headers);
                if (!resp || resp->status != httplib::OK_200)
                {
                    resultEmitter->InvokeAll({ resp ? resp->status : -1, "Failed to retrieve beatmap user score", std::nullopt });
                    return;
                }

                if (resp->body.empty())
                {
                    resultEmitter->InvokeAll({ httplib::OK_200, "", std::nullopt });
                    return;
                }

                try
                {
                    zjson::JsonParser parser(resp->body);

                    resp::BeatmapUserScore score{};
                    if (!parser.IsNull(".pp"))
                        score.pp = parser.GetFloat(".pp");
                    score.isBest = parser.GetBool(".isBest");
                    resultEmitter->InvokeAll({ httplib::OK_200, "", score });
                }
                catch (zjson::ParseException e)
                {
                    resultEmitter->InvokeAll({ 0, "Beatmap user score parsing failed: " + std::string(e.what()), std::nullopt });
                }
            }).detach();

            return subscription;
        }
    };
}