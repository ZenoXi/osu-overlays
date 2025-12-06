#pragma once

#include <functional>
#include <string>
#include <optional>
#include <any>

#include "decimal.h"
#include "Value.h"
#include "UICore/Model/Color.h"

struct ValueProxy
{
    enum ValueType
    {
        TEXT,
        NUMBER,
        SELECTION,
        BOOL,
        COLOR
    };
    typedef dec::decimal<7> Number;

    template<typename T>
    struct Mapper
    {
        std::function<std::optional<T>(const std::any&)> fromValue;
        std::function<bool(const std::optional<T>&, std::any&)> toValue;
    };

    std::string name;
    std::any valuePtr;
    ValueType type = TEXT;
    bool optional = false;
    bool computed = false;

    std::optional<Mapper<std::wstring>> textMapper = std::nullopt;
    std::optional<Mapper<Number>> numberMapper = std::nullopt;
    std::optional<Mapper<int64_t>> selectionMapper = std::nullopt;
    std::optional<Mapper<bool>> boolMapper = std::nullopt;
    std::optional<Mapper<zcom::Color>> colorMapper = std::nullopt;

    int numberPrecision = 0;
    Number numberMinValue = Number(std::numeric_limits<int>::min());
    Number numberMaxValue = Number(std::numeric_limits<int>::max());
    Number numberStepSize = Number(1);

    std::vector<std::pair<int64_t, std::wstring>> selectionValues;
    bool dropdownAbove = false;

    ValueProxy& Computed()
    {
        computed = true;
        return *this;
    }

    static ValueProxy BasicTextValueProxy(std::string name, std::any valuePtr)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = TEXT;
        proxy.optional = false;
        proxy.textMapper = Mapper<std::wstring>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::wstring>*>(valuePtr);
                return std::optional<std::wstring>(ptr->Get());
            },
            .toValue = [](const std::optional<std::wstring>& text, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::wstring>*>(valuePtr);
                ptr->Assign(text.value());
                return true;
            }
        };
        return proxy;
    }
    static ValueProxy BasicOptionalTextValueProxy(std::string name, std::any valuePtr)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = TEXT;
        proxy.optional = true;
        proxy.textMapper = Mapper<std::wstring>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<std::wstring>>*>(valuePtr);
                return ptr->Get();
            },
            .toValue = [](const std::optional<std::wstring>& text, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<std::wstring>>*>(valuePtr);
                ptr->Assign(text);
                return true;
            }
        };
        return proxy;
    }

    template<typename _Int>
    static ValueProxy BasicIntValueProxy(std::string name, std::any valuePtr, Number minValue = Number(std::numeric_limits<int>::min()), Number maxValue = Number(std::numeric_limits<int>::max()), Number stepSize = Number(1))
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = NUMBER;
        proxy.optional = false;
        proxy.numberMinValue = minValue;
        proxy.numberMaxValue = maxValue;
        proxy.numberStepSize = stepSize;
        proxy.numberMapper = Mapper<Number>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<_Int>*>(valuePtr);
                return std::optional<Number>(Number(ptr->Get()));
            },
            .toValue = [](const std::optional<Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<_Int>*>(valuePtr);
                ptr->Assign((_Int)number->getAsInteger());
                return true;
            }
        };
        return proxy;
    }
    template<typename _Int>
    static ValueProxy BasicOptionalIntValueProxy(std::string name, std::any valuePtr, Number minValue = Number(std::numeric_limits<int>::min()), Number maxValue = Number(std::numeric_limits<int>::max()), Number stepSize = Number(1))
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = NUMBER;
        proxy.optional = true;
        proxy.numberMinValue = minValue;
        proxy.numberMaxValue = maxValue;
        proxy.numberStepSize = stepSize;
        proxy.numberMapper = Mapper<Number>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<_Int>>*>(valuePtr);
                return ptr->Get() ? std::optional<Number>(Number(ptr->Get().value())) : std::nullopt;
            },
            .toValue = [](const std::optional<Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<_Int>>*>(valuePtr);
                ptr->Assign(number ? std::optional<_Int>((_Int)number->getAsInteger()) : std::nullopt);
                return true;
            }
        };
        return proxy;
    }

    template<typename _Float>
    static ValueProxy BasicFloatValueProxy(std::string name, std::any valuePtr, int precision, Number minValue = Number(std::numeric_limits<int>::min()), Number maxValue = Number(std::numeric_limits<int>::max()), Number stepSize = Number(1))
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = NUMBER;
        proxy.optional = false;
        proxy.numberPrecision = precision;
        proxy.numberMinValue = minValue;
        proxy.numberMaxValue = maxValue;
        proxy.numberStepSize = stepSize;
        proxy.numberMapper = Mapper<Number>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<_Float>*>(valuePtr);
                return std::optional<Number>(Number(ptr->Get()));
            },
            .toValue = [](const std::optional<Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<_Float>*>(valuePtr);
                ptr->Assign((_Float)number->getAsDouble());
                return true;
            }
        };
        return proxy;
    }
    template<typename _Float>
    static ValueProxy BasicOptionalFloatValueProxy(std::string name, std::any valuePtr, int precision, Number minValue = Number(std::numeric_limits<int>::min()), Number maxValue = Number(std::numeric_limits<int>::max()), Number stepSize = Number(1))
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = NUMBER;
        proxy.optional = true;
        proxy.numberPrecision = precision;
        proxy.numberMinValue = minValue;
        proxy.numberMaxValue = maxValue;
        proxy.numberStepSize = stepSize;
        proxy.numberMapper = Mapper<Number>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<_Float>>*>(valuePtr);
                return ptr->Get() ? std::optional<Number>(Number(ptr->Get().value())) : std::nullopt;
            },
            .toValue = [](const std::optional<Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<_Float>>*>(valuePtr);
                ptr->Assign(number ? std::optional<_Float>((_Float)number->getAsDouble()) : std::nullopt);
                return true;
            }
        };
        return proxy;
    }

    static ValueProxy BasicNumberValueProxy(std::string name, std::any valuePtr, int precision, Number minValue = Number(std::numeric_limits<int>::min()), Number maxValue = Number(std::numeric_limits<int>::max()), Number stepSize = Number(1))
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = NUMBER;
        proxy.optional = false;
        proxy.numberPrecision = precision;
        proxy.numberMinValue = minValue;
        proxy.numberMaxValue = maxValue;
        proxy.numberStepSize = stepSize;
        proxy.numberMapper = Mapper<Number>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<Number>*>(valuePtr);
                return std::optional<Number>(ptr->Get());
            },
            .toValue = [](const std::optional<Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<Number>*>(valuePtr);
                ptr->Assign(number.value());
                return true;
            }
        };
        return proxy;
    }
    static ValueProxy BasicOptionalNumberValueProxy(std::string name, std::any valuePtr, int precision, Number minValue = Number(std::numeric_limits<int>::min()), Number maxValue = Number(std::numeric_limits<int>::max()), Number stepSize = Number(1))
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = NUMBER;
        proxy.optional = true;
        proxy.numberPrecision = precision;
        proxy.numberMinValue = minValue;
        proxy.numberMaxValue = maxValue;
        proxy.numberStepSize = stepSize;
        proxy.numberMapper = Mapper<Number>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<Number>>*>(valuePtr);
                return ptr->Get();
            },
            .toValue = [](const std::optional<Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<Number>>*>(valuePtr);
                ptr->Assign(number);
                return true;
            }
        };
        return proxy;
    }

    template<typename _Enum>
    static ValueProxy BasicEnumValueProxy(std::string name, std::any valuePtr, const std::vector<std::pair<int64_t, std::wstring>>& selectionValues, bool dropdownAbove = false)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = SELECTION;
        proxy.optional = false;
        proxy.selectionValues = selectionValues;
        proxy.dropdownAbove = dropdownAbove;
        proxy.selectionMapper = Mapper<int64_t>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<_Enum>*>(valuePtr);
                return std::optional<int64_t>((int64_t)ptr->Get());
            },
            .toValue = [](const std::optional<int64_t>& value, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<_Enum>*>(valuePtr);
                ptr->Assign((_Enum)value.value());
                return true;
            }
        };
        return proxy;
    }

    template<typename _Enum>
    static ValueProxy BasicOptionalEnumValueProxy(std::string name, std::any valuePtr, const std::vector<std::pair<int64_t, std::wstring>>& selectionValues, bool dropdownAbove = false)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = SELECTION;
        proxy.optional = true;
        proxy.selectionValues = selectionValues;
        proxy.dropdownAbove = dropdownAbove;
        proxy.selectionMapper = Mapper<int64_t>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<_Enum>>*>(valuePtr);
                return ptr->Get() ? std::optional<int64_t>((int64_t)ptr->Get().value()) : std::nullopt;
            },
            .toValue = [](const std::optional<int64_t>& value, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<_Enum>>*>(valuePtr);
                ptr->Assign(value ? std::optional<_Enum>((_Enum)value.value()) : std::nullopt);
                return true;
            }
        };
        return proxy;
    }

    static ValueProxy BasicBoolValueProxy(std::string name, std::any valuePtr)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = BOOL;
        proxy.optional = false;
        proxy.boolMapper = Mapper<bool>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<bool>*>(valuePtr);
                return std::optional<bool>(ptr->Get());
            },
            .toValue = [](const std::optional<bool>& value, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<bool>*>(valuePtr);
                ptr->Assign(value.value());
                return true;
            }
        };
        return proxy;
    }
    static ValueProxy BasicOptionalBoolValueProxy(std::string name, std::any valuePtr)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = BOOL;
        proxy.optional = true;
        proxy.boolMapper = Mapper<bool>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<bool>>*>(valuePtr);
                return ptr->Get();
            },
            .toValue = [](const std::optional<bool>& value, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<bool>>*>(valuePtr);
                ptr->Assign(value);
                return true;
            }
        };
        return proxy;
    }

    static ValueProxy BasicColorValueProxy(std::string name, std::any valuePtr)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = COLOR;
        proxy.optional = false;
        proxy.colorMapper = Mapper<zcom::Color>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<zcom::Color>*>(valuePtr);
                return std::optional<zcom::Color>(ptr->Get());
            },
            .toValue = [](const std::optional<zcom::Color>& value, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<zcom::Color>*>(valuePtr);
                ptr->Assign(value.value());
                return true;
            }
        };
        return proxy;
    }
    static ValueProxy BasicOptionalColorValueProxy(std::string name, std::any valuePtr)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = COLOR;
        proxy.optional = true;
        proxy.colorMapper = Mapper<zcom::Color>{
            .fromValue = [](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<zcom::Color>>*>(valuePtr);
                return ptr->Get();
            },
            .toValue = [](const std::optional<zcom::Color>& value, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<std::optional<zcom::Color>>*>(valuePtr);
                ptr->Assign(value);
                return true;
            }
        };
        return proxy;
    }
};