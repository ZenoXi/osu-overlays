#include "Options.h"

#include "UICore/Helper/StringHelper.h"

#include <array>
#include <sstream>
#include <fstream>
#include <filesystem>

std::optional<std::wstring> Options::GetValue(const std::wstring& name)
{
    std::lock_guard<std::mutex> lock(_m_options);
    auto it = _options.find(name);
    if (it != _options.end())
        return it->second;
    else
        return std::nullopt;
}

std::optional<int> Options::GetIntValue(const std::wstring& name)
{
    auto strOpt = GetValue(name);
    if (!strOpt)
        return std::nullopt;

    try {
        return std::stoi(strOpt.value());
    }
    catch (std::exception) {
        return std::nullopt;
    }
}

std::optional<double> Options::GetDoubleValue(const std::wstring& name)
{
    auto strOpt = GetValue(name);
    if (!strOpt)
        return std::nullopt;

    try {
        return std::stod(strOpt.value());
    }
    catch (std::exception) {
        return std::nullopt;
    }
}

void Options::SetValue(const std::wstring& name, const std::wstring& value, bool save)
{
    std::lock_guard<std::mutex> lock(_m_options);
    _options[name] = value;
    if (save)
        _SaveToFile();
}

void Options::SetIntValue(const std::wstring& name, int value, bool save)
{
    std::wostringstream ss(L"");
    ss << value;
    SetValue(name, ss.str(), save);
}

void Options::SetDoubleValue(const std::wstring& name, double value, bool save)
{
    std::wostringstream ss(L"");
    ss << value;
    SetValue(name, ss.str(), save);
}

void Options::LoadOptions()
{
    std::lock_guard<std::mutex> lock(_m_options);
    _LoadFromFile();
}

void Options::SaveOptions()
{
    std::lock_guard<std::mutex> lock(_m_options);
    _SaveToFile();
}

void Options::_LoadFromFile()
{
    _options.clear();

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

        _options[parts[0]] = parts[1];
    }
}

void Options::_SaveToFile()
{
    std::wofstream fout(_filePath);
    if (!fout)
        return;

    for (auto& option : _options)
        fout << option.first << '=' << option.second << std::endl;
}