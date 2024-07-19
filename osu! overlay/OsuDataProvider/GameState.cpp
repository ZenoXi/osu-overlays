#include "GameState.h"

void osu::ParseJson(osu::GameState& state, const std::string& inputStr)
{
    using json = nlohmann::json;
    json rootObject = json::parse(inputStr);

    if (rootObject.contains("menu"))
    {
        json menuObject = rootObject["menu"];
        if (menuObject.contains("state"))
            menuObject.at("state").get_to(state.menuState.state);
        if (menuObject.contains("skinFolder"))
            menuObject.at("skinFolder").get_to(state.menuState.skinFolder);
        if (menuObject.contains("gameMode"))
            menuObject.at("gameMode").get_to(state.menuState.gameMode);
        if (menuObject.contains("isChatEnabled"))
            menuObject.at("isChatEnabled").get_to(state.menuState.isChatEnabled);
        if (menuObject.contains("bm"))
        {
            json beatmapObject = menuObject.at("bm");
            if (beatmapObject.contains("time"))
            {
                json timeObject = beatmapObject.at("time");
                if (timeObject.contains("firstObj"))
                    state.menuState.beatmap.firstObjectTime = TimePoint(timeObject.at("firstObj").get<int>(), MILLISECONDS);
                if (timeObject.contains("current"))
                    state.menuState.beatmap.currentTime = TimePoint(timeObject.at("current").get<int>(), MILLISECONDS);
                if (timeObject.contains("full"))
                    state.menuState.beatmap.lastObjectTime = TimePoint(timeObject.at("full").get<int>(), MILLISECONDS);
                if (timeObject.contains("mp3"))
                    state.menuState.beatmap.mp3Length = Duration(timeObject.at("mp3").get<int>(), MILLISECONDS);
            }
            if (beatmapObject.contains("id"))
                beatmapObject.at("id").get_to(state.menuState.beatmap.mapId);
            if (beatmapObject.contains("set"))
                beatmapObject.at("set").get_to(state.menuState.beatmap.setId);
            if (beatmapObject.contains("md5"))
                beatmapObject.at("md5").get_to(state.menuState.beatmap.md5);
            if (beatmapObject.contains("rankedStatus"))
                beatmapObject.at("rankedStatus").get_to(state.menuState.beatmap.rankedStatus);
            if (beatmapObject.contains("metadata"))
            {
                json metadataObject = beatmapObject.at("metadata");
                if (metadataObject.contains("artist"))
                    metadataObject.at("artist").get_to(state.menuState.beatmap.artist);
                if (metadataObject.contains("title"))
                    metadataObject.at("title").get_to(state.menuState.beatmap.title);
                if (metadataObject.contains("mapper"))
                    metadataObject.at("mapper").get_to(state.menuState.beatmap.mapper);
                if (metadataObject.contains("difficulty"))
                    metadataObject.at("difficulty").get_to(state.menuState.beatmap.difficultyName);
            }
            if (beatmapObject.contains("stats"))
            {
                json statsObject = beatmapObject.at("stats");
                if (statsObject.contains("AR"))
                    statsObject.at("AR").get_to(state.menuState.beatmap.AR);
                if (statsObject.contains("CS"))
                    statsObject.at("CS").get_to(state.menuState.beatmap.CS);
                if (statsObject.contains("OD"))
                    statsObject.at("OD").get_to(state.menuState.beatmap.OD);
                if (statsObject.contains("HP"))
                    statsObject.at("HP").get_to(state.menuState.beatmap.HP);
                if (statsObject.contains("SR"))
                    statsObject.at("SR").get_to(state.menuState.beatmap.currentSR);
                if (statsObject.contains("BPM"))
                {
                    json bpmObject = statsObject.at("BPM");
                    if (bpmObject.contains("min"))
                        bpmObject.at("min").get_to(state.menuState.beatmap.minBPM);
                    if (bpmObject.contains("max"))
                        bpmObject.at("max").get_to(state.menuState.beatmap.maxBPM);
                }
                if (statsObject.contains("fullSR"))
                    statsObject.at("fullSR").get_to(state.menuState.beatmap.baseSR);
                if (statsObject.contains("memoryAR"))
                    statsObject.at("memoryAR").get_to(state.menuState.beatmap.baseAR);
                if (statsObject.contains("memoryCS"))
                    statsObject.at("memoryCS").get_to(state.menuState.beatmap.baseCS);
                if (statsObject.contains("memoryOD"))
                    statsObject.at("memoryOD").get_to(state.menuState.beatmap.baseOD);
                if (statsObject.contains("memoryHP"))
                    statsObject.at("memoryHP").get_to(state.menuState.beatmap.baseHP);
            }
            if (beatmapObject.contains("path"))
            {
                json pathObject = beatmapObject.at("path");
                if (pathObject.contains("full"))
                    pathObject.at("full").get_to(state.menuState.beatmap.fullPath);
                if (pathObject.contains("folder"))
                    pathObject.at("folder").get_to(state.menuState.beatmap.folderPath);
                if (pathObject.contains("file"))
                    pathObject.at("file").get_to(state.menuState.beatmap.filePath);
                if (pathObject.contains("bg"))
                    pathObject.at("bg").get_to(state.menuState.beatmap.backgroundFilename);
                if (pathObject.contains("audio"))
                    pathObject.at("audio").get_to(state.menuState.beatmap.audioFilename);
            }
        }
        if (menuObject.contains("mods"))
        {
            json modsObject = menuObject.at("mods");
            if (modsObject.contains("num"))
                modsObject.at("num").get_to(state.menuState.modNumber);
            if (modsObject.contains("str"))
                modsObject.at("str").get_to(state.menuState.modString);
        }
        if (menuObject.contains("pp"))
        {
            json ppObject = menuObject.at("pp");
            if (ppObject.contains("100"))
                ppObject.at("100").get_to(state.menuState.ppFor100);
            if (ppObject.contains("99"))
                ppObject.at("99").get_to(state.menuState.ppFor99);
            if (ppObject.contains("98"))
                ppObject.at("98").get_to(state.menuState.ppFor98);
            if (ppObject.contains("97"))
                ppObject.at("97").get_to(state.menuState.ppFor97);
            if (ppObject.contains("96"))
                ppObject.at("96").get_to(state.menuState.ppFor96);
            if (ppObject.contains("95"))
                ppObject.at("95").get_to(state.menuState.ppFor95);
            if (ppObject.contains("strains"))
            {
                json strainsObject = ppObject.at("strains");
                if (strainsObject.is_array())
                {
                    state.menuState.strains.resize(strainsObject.size());
                    int index = 0;
                    for (json value : strainsObject)
                        state.menuState.strains[index++] = value.get<float>();
                }
            }
        }
    }
    if (rootObject.contains("gameplay"))
    {
        json gameplayObject = rootObject.at("gameplay");
        if (gameplayObject.contains("gameMode"))
            gameplayObject.at("gameMode").get_to(state.gameplayState.gameMode);
        if (gameplayObject.contains("name"))
            gameplayObject.at("name").get_to(state.gameplayState.playerName);
        if (gameplayObject.contains("score"))
            gameplayObject.at("score").get_to(state.gameplayState.score);
        if (gameplayObject.contains("accuracy"))
            gameplayObject.at("accuracy").get_to(state.gameplayState.accuracy);
        if (gameplayObject.contains("combo"))
        {
            json comboObject = gameplayObject.at("combo");
            if (comboObject.contains("current"))
                comboObject.at("current").get_to(state.gameplayState.currentCombo);
            if (comboObject.contains("hp"))
                comboObject.at("hp").get_to(state.gameplayState.maxCombo);
        }
        if (gameplayObject.contains("hits"))
        {
            json hitsObject = gameplayObject.at("hits");
            if (hitsObject.contains("300"))
                hitsObject.at("300").get_to(state.gameplayState.count300);
            if (hitsObject.contains("200"))
                hitsObject.at("200").get_to(state.gameplayState.count200);
            if (hitsObject.contains("geki"))
                hitsObject.at("geki").get_to(state.gameplayState.countGeki);
            if (hitsObject.contains("100"))
                hitsObject.at("100").get_to(state.gameplayState.count100);
            if (hitsObject.contains("katu"))
                hitsObject.at("katu").get_to(state.gameplayState.countKatu);
            if (hitsObject.contains("50"))
                hitsObject.at("50").get_to(state.gameplayState.count50);
            if (hitsObject.contains("0"))
                hitsObject.at("0").get_to(state.gameplayState.countMiss);
            if (hitsObject.contains("sliderBreaks"))
                hitsObject.at("sliderBreaks").get_to(state.gameplayState.countSliderbreak);
            if (hitsObject.contains("grade"))
            {
                json gradeObject = hitsObject.at("grade");
                if (gradeObject.contains("current"))
                    gradeObject.at("current").get_to(state.gameplayState.currentGrade);
                if (gradeObject.contains("maxThisPlay"))
                    gradeObject.at("maxThisPlay").get_to(state.gameplayState.maxGradeThisPlay);
            }
            if (hitsObject.contains("unstableRate"))
                hitsObject.at("unstableRate").get_to(state.gameplayState.unstableRate);
            if (hitsObject.contains("hitErrorArray"))
            {
                json hitErrorArrayObject = hitsObject.at("hitErrorArray");
                if (hitErrorArrayObject.is_array())
                {
                    state.gameplayState.hitErrorArray.resize(hitErrorArrayObject.size());
                    int index = 0;
                    for (json value : hitErrorArrayObject)
                        state.gameplayState.hitErrorArray[index++] = value.get<int>();
                }
            }
        }
        if (gameplayObject.contains("pp"))
        {
            json ppObject = gameplayObject.at("pp");
            if (ppObject.contains("current"))
                ppObject.at("current").get_to(state.gameplayState.currentPP);
            if (ppObject.contains("fc"))
                ppObject.at("fc").get_to(state.gameplayState.PPforFC);
            if (ppObject.contains("maxThisPlay"))
                ppObject.at("maxThisPlay").get_to(state.gameplayState.maxPPForThisPlay);
        }
        if (gameplayObject.contains("leaderboard"))
        {
            json leaderboardObject = gameplayObject.at("leaderboard");
            if (leaderboardObject.contains("hasLeaderboard"))
                leaderboardObject.at("hasLeaderboard").get_to(state.gameplayState.leaderboard.hasLeaderboard);
            if (leaderboardObject.contains("ourplayer"))
            {
                LeaderboardItem currentPlayer{};
                json playerObject = leaderboardObject.at("ourplayer");
                if (playerObject.contains("name"))
                    playerObject.at("name").get_to(currentPlayer.username);
                if (playerObject.contains("score"))
                    playerObject.at("score").get_to(currentPlayer.score);
                if (playerObject.contains("combo"))
                    playerObject.at("combo").get_to(currentPlayer.combo);
                if (playerObject.contains("maxCombo"))
                    playerObject.at("maxCombo").get_to(currentPlayer.maxCombo);
                if (playerObject.contains("mods"))
                    playerObject.at("mods").get_to(currentPlayer.mods);
                if (playerObject.contains("h300"))
                    playerObject.at("h300").get_to(currentPlayer.count300);
                if (playerObject.contains("h100"))
                    playerObject.at("h100").get_to(currentPlayer.count100);
                if (playerObject.contains("h50"))
                    playerObject.at("h50").get_to(currentPlayer.count50);
                if (playerObject.contains("h0"))
                    playerObject.at("h0").get_to(currentPlayer.countMiss);
                if (playerObject.contains("team"))
                    playerObject.at("team").get_to(currentPlayer.team);
                if (playerObject.contains("position"))
                    playerObject.at("position").get_to(currentPlayer.position);
                if (playerObject.contains("isPassing"))
                    playerObject.at("isPassing").get_to(currentPlayer.isPassing);
                state.gameplayState.leaderboard.currentPlayer = currentPlayer;
            }
            if (leaderboardObject.contains("slots"))
            {
                json slotsObject = leaderboardObject.at("slots");
                if (slotsObject.is_array())
                {
                    state.gameplayState.leaderboard.players.resize(slotsObject.size());
                    int index = 0;
                    for (json slotObject : slotsObject)
                    {
                        LeaderboardItem player;
                        if (slotObject.contains("name"))
                            slotObject.at("name").get_to(player.username);
                        if (slotObject.contains("score"))
                            slotObject.at("score").get_to(player.score);
                        if (slotObject.contains("combo"))
                            slotObject.at("combo").get_to(player.combo);
                        if (slotObject.contains("maxCombo"))
                            slotObject.at("maxCombo").get_to(player.maxCombo);
                        if (slotObject.contains("mods"))
                            slotObject.at("mods").get_to(player.mods);
                        if (slotObject.contains("h300"))
                            slotObject.at("h300").get_to(player.count300);
                        if (slotObject.contains("h100"))
                            slotObject.at("h100").get_to(player.count100);
                        if (slotObject.contains("h50"))
                            slotObject.at("h50").get_to(player.count50);
                        if (slotObject.contains("h0"))
                            slotObject.at("h0").get_to(player.countMiss);
                        if (slotObject.contains("team"))
                            slotObject.at("team").get_to(player.team);
                        if (slotObject.contains("position"))
                            slotObject.at("position").get_to(player.position);
                        if (slotObject.contains("isPassing"))
                            slotObject.at("isPassing").get_to(player.isPassing);
                        state.gameplayState.leaderboard.players[index++] = player;
                    }
                }
            }
        }
    }
}