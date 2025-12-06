#include "GameState.h"

#include "Shared/Util/JsonParser.h"

#include <iostream>

void osu::ParseJson(osu::GameState& state, const std::string& inputStr)
{
    try
    {
        //SimpleTimer timer;
        zjson::JsonParser parser(inputStr);
        state.client = parser.GetString(".client");
        state.server = parser.GetString(".server");
        state.state.number = parser.GetInt(".state.number");
        state.state.name = parser.GetString(".state.name");
        state.settings.mode.number = parser.GetInt(".settings.mode.number");
        state.settings.mode.name = parser.GetString(".settings.mode.name");
        state.profile.banchoStatus.number = parser.GetInt(".profile.banchoStatus.number");
        state.profile.banchoStatus.name = parser.GetString(".profile.banchoStatus.name");
        state.profile.id = parser.GetInt(".profile.id");
        state.profile.name = parser.GetString(".profile.name");
        state.profile.mode.number = parser.GetInt(".profile.mode.number");
        state.profile.mode.name = parser.GetString(".profile.mode.name");
        state.beatmap.time.live = TimePoint(parser.GetLong(".beatmap.time.live"), MILLISECONDS);
        state.beatmap.time.firstObject = TimePoint(parser.GetLong(".beatmap.time.firstObject"), MILLISECONDS);
        state.beatmap.time.lastObject = TimePoint(parser.GetLong(".beatmap.time.lastObject"), MILLISECONDS);
        state.beatmap.time.mp3Length = Duration(parser.GetLong(".beatmap.time.mp3Length"), MILLISECONDS);
        state.beatmap.status.number = parser.GetInt(".beatmap.status.number");
        state.beatmap.status.name = parser.GetString(".beatmap.status.name");
        state.beatmap.id = parser.GetUnparsedValue(".beatmap.id");
        state.beatmap.mode.number = parser.GetInt(".beatmap.mode.number");
        state.beatmap.mode.name = parser.GetString(".beatmap.mode.name");
        state.play.playerName = parser.GetString(".play.playerName");
        state.play.mode.number = parser.GetInt(".play.mode.number");
        state.play.mode.name = parser.GetString(".play.mode.name");
        state.play.score = parser.GetLong(".play.score");
        state.play.accuracy = (float)parser.GetDouble(".play.accuracy");
        state.play.hits.count300 = parser.GetInt(".play.hits.300");
        state.play.hits.count100 = parser.GetInt(".play.hits.100");
        state.play.hits.count50 = parser.GetInt(".play.hits.50");
        state.play.hits.countMiss = parser.GetInt(".play.hits.0");
        state.play.combo.current = parser.GetInt(".play.combo.current");
        state.play.mods.rate = (float)parser.GetDouble(".play.mods.rate");
        state.play.pp.current = (float)parser.GetDouble(".play.pp.current");
        state.play.unstableRate = (float)parser.GetDouble(".play.unstableRate");

        size_t strainCount = parser.GetArraySize(".performance.graph.xaxis");
        for (size_t i = 0; i < strainCount; i++)
        {
            std::string index = std::to_string(i);
            state.performance.graph.aim.push_back((float)parser.GetDouble(".performance.graph.series[0].data[" + index + "]"));
            state.performance.graph.aimNoSliders.push_back((float)parser.GetDouble(".performance.graph.series[1].data[" + index + "]"));
            state.performance.graph.flashlight.push_back((float)parser.GetDouble(".performance.graph.series[2].data[" + index + "]"));
            state.performance.graph.speed.push_back((float)parser.GetDouble(".performance.graph.series[3].data[" + index + "]"));
            state.performance.graph.xaxis.push_back(parser.GetLong(".performance.graph.xaxis[" + index + "]"));
        }

        state.resultsScreen.scoreId = parser.GetLong(".resultsScreen.scoreId");
        state.resultsScreen.playerName = parser.GetString(".resultsScreen.playerName");
        state.resultsScreen.pp.current = (float)parser.GetDouble(".resultsScreen.pp.current");
        state.resultsScreen.createdAt = parser.GetString(".resultsScreen.createdAt");
    }
    catch (zjson::ParseException e)
    {
        std::cout << e.what() << " at " << e.ErrorPos() << '\n';
    }
    catch (std::out_of_range e)
    {
        std::cout << e.what() << '\n';
    }
    catch (std::invalid_argument e)
    {
        std::cout << e.what() << '\n';
    }
}