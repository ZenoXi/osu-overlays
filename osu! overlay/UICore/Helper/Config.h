#pragma once

#include <map>
#include <string>
#include <mutex>
#include <optional>

#include "ConfigValue.h"
#include "EventEmitter.h"

class Config
{
public:
    enum FetchOptions
    {
        NONE,
        ADD_IF_MISSING,
        ADD_AND_SAVE_IF_MISSING
    };

    std::optional<std::wstring> GetValue(const std::wstring& name);
    std::optional<int64_t> GetIntValue(const std::wstring& name);
    std::optional<double> GetDoubleValue(const std::wstring& name);
    void SetValue(const std::wstring& name, const std::wstring& value, bool save = true);
    void SetIntValue(const std::wstring& name, int64_t value, bool save = true);
    void SetDoubleValue(const std::wstring& name, double value, bool save = true);
    void LoadConfig();
    void SaveConfig();

    std::unique_ptr<AsyncEventSubscription<void, std::optional<std::pair<std::wstring, std::wstring>>>> SubscribeOnConfigValueChanged()
    {
        return _configValueChangedEventEmitter->SubscribeAsync();
    }

    std::wstring GetConfigValue(const ConfigValue<std::wstring>& configValue, FetchOptions options = NONE)
    {
        auto valueOptional = GetValue(configValue.name);
        if (!valueOptional)
        {
            if (options == ADD_IF_MISSING)
                SetValue(configValue.name, configValue.defaultValue, false);
            else if (options == ADD_AND_SAVE_IF_MISSING)
                SetValue(configValue.name, configValue.defaultValue, true);
        }
        return valueOptional.value_or(configValue.defaultValue);
    }

    template<typename T>
    T GetIntConfigValue(const ConfigValue<T>& configValue, FetchOptions options = NONE)
    {
        auto valueOptional = GetIntValue(configValue.name);
        if (!valueOptional)
        {
            if (options == ADD_IF_MISSING)
                SetIntValue(configValue.name, configValue.defaultValue, false);
            else if (options == ADD_AND_SAVE_IF_MISSING)
                SetIntValue(configValue.name, configValue.defaultValue, true);
        }
        return (T)valueOptional.value_or(configValue.defaultValue);
    }

    template<typename T>
    T GetDoubleConfigValue(const ConfigValue<T>& configValue, FetchOptions options = NONE)
    {
        auto valueOptional = GetDoubleValue(configValue.name);
        if (!valueOptional)
        {
            if (options == ADD_IF_MISSING)
                SetDoubleValue(configValue.name, configValue.defaultValue, false);
            else if (options == ADD_AND_SAVE_IF_MISSING)
                SetDoubleValue(configValue.name, configValue.defaultValue, true);
        }
        return (T)valueOptional.value_or(configValue.defaultValue);
    }

    Config(std::wstring filePath) : _filePath(filePath) {}

private:
    std::wstring _filePath;
    std::map<std::wstring, std::wstring> _values;
    std::mutex _m_values;

    EventEmitter<void, std::optional<std::pair<std::wstring, std::wstring>>> _configValueChangedEventEmitter = EventEmitter<void, std::optional<std::pair<std::wstring, std::wstring>>>(EventEmitterThreadMode::MULTITHREADED);

    void _LoadFromFile();
    void _SaveToFile();
};