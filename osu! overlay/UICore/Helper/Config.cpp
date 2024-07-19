#include "Config.h"

#include "UICore/Helper/StringHelper.h"

#include <array>
#include <sstream>
#include <fstream>
#include <filesystem>

std::optional<std::wstring> Config::GetValue(const std::wstring& name)
{
    std::lock_guard<std::mutex> lock(_m_values);
    auto it = _values.find(name);
    if (it != _values.end())
        return it->second;
    else
        return std::nullopt;
}

std::optional<int64_t> Config::GetIntValue(const std::wstring& name)
{
    auto strOpt = GetValue(name);
    if (!strOpt)
        return std::nullopt;

    try {
        return std::stoll(strOpt.value());
    } catch (std::exception) {
        return std::nullopt;
    }
}

std::optional<double> Config::GetDoubleValue(const std::wstring& name)
{
    auto strOpt = GetValue(name);
    if (!strOpt)
        return std::nullopt;

    try {
        return std::stod(strOpt.value());
    } catch (std::exception) {
        return std::nullopt;
    }
}

void Config::SetValue(const std::wstring& name, const std::wstring& value, bool save)
{
    std::lock_guard<std::mutex> lock(_m_values);
    _values[name] = value;
    _configValueChangedEventEmitter->InvokeAll(std::pair(name, value));
    if (save)
        _SaveToFile();
}

void Config::SetIntValue(const std::wstring& name, int64_t value, bool save)
{
    std::wostringstream ss(L"");
    ss << value;
    SetValue(name, ss.str(), save);
}

void Config::SetDoubleValue(const std::wstring& name, double value, bool save)
{
    std::wostringstream ss(L"");
    ss << value;
    SetValue(name, ss.str(), save);
}

void Config::LoadConfig()
{
    std::lock_guard<std::mutex> lock(_m_values);
    _LoadFromFile();
}

void Config::SaveConfig()
{
    std::lock_guard<std::mutex> lock(_m_values);
    _SaveToFile();
}

void Config::_LoadFromFile()
{
    // Locking is done from outside of this function 
    _values.clear();

    std::wifstream fin(_filePath);
    if (!fin)
        return;

    while (!fin.eof())
    {
        std::wstring line;
        std::getline(fin, line);
        
        std::array<std::wstring, 2> parts;
        split_wstr(line, parts, '=');

        if (parts[0].empty())
            continue;

        _values[parts[0]] = parts[1];
    }
    _configValueChangedEventEmitter->InvokeAll(std::nullopt);
}

void Config::_SaveToFile()
{
    // Locking is done from outside of this function 
    std::wofstream fout(_filePath);
    if (!fout)
        return;

    for (auto& option : _values)
        fout << option.first << '=' << option.second << std::endl;
}