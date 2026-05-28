#pragma once

#include <vector>
#include <optional>
#include <string>
#include <mutex>

class GPUDetector
{
public:
    enum class GPUType
    {
        UNKNOWN,
        NVIDIA,
        AMD,
        INTEL
    };

    static void UpdateGPUInfo();
    static std::optional<GPUType> GetPrimaryGPUType();
    static bool AtLeastOneOfType(GPUType type);
    static bool AllOfType(GPUType type);
    static std::vector<GPUType> GetDetectedGPUs();

private:
    static std::vector<GPUType> _detectedGPUs;
    static std::mutex _mtx;
};

std::string GPUTypeToString(GPUDetector::GPUType type);