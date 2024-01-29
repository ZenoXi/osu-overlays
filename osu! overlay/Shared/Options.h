#pragma once

#include <map>
#include <string>
#include <mutex>
#include <optional>

class Options
{
public:
    std::optional<std::wstring> GetValue(const std::wstring& name);
    std::optional<int> GetIntValue(const std::wstring& name);
    std::optional<double> GetDoubleValue(const std::wstring& name);
    void SetValue(const std::wstring& name, const std::wstring& value, bool save = true);
    void SetIntValue(const std::wstring& name, int value, bool save = true);
    void SetDoubleValue(const std::wstring& name, double value, bool save = true);
    void LoadOptions();
    void SaveOptions();

    Options(std::wstring filePath) : _filePath(filePath) {}

private:
    std::wstring _filePath;
    std::map<std::wstring, std::wstring> _options;
    std::mutex _m_options;

    void _LoadFromFile();
    void _SaveToFile();
};