#pragma once

struct SmokeSimParams
{
    int trailWidth;
    int trailEdgeFadeRange;
    float trailDensity;
    int trailWindWidth;
    float trailWindSpeed;
    float cursorTemp;
    int trailColor;
    float trailVelocityDiffusion;
    float trailDensityDiffusion;
    float trailTemperatureDiffusion;
    float trailDensityReductionRate;
    float trailTemperatureReductionRate;
    int smokeColor;
    int brushWidth;
    int brushEdgeFadeRange;
    float smokeDensity;
    int cursorWindWidth;
    float cursorWindSpeed;
    int slowdownPersistenceDurationMs;
    float smokeVelocityDiffusion;
    float smokeDensityDiffusion;
    float smokeDensityReductionRate;
    int smokeKeyCode;
};