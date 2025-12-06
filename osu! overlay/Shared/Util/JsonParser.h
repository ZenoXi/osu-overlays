#pragma once

#include <string>
#include <vector>
#include <optional>
#include <exception>

namespace zjson
{
    class ParseException : public std::exception
    {
        std::string _msg;
        size_t _pos;

    public:
        ParseException(const char* msg, size_t pos)
        {
            _msg = msg;
            _pos = pos;
        }

        size_t ErrorPos() const
        {
            return _pos;
        }

        const char* what() const throw()
        {
            return _msg.c_str();
        }
    };

    class JsonParser
    {
    public:
        enum Type
        {
            ROOT,
            OBJECT,
            ARRAY,
            VALUE
        };

        JsonParser(const std::string& json)
        {
            _jsonSrc = json;
            _json = _jsonSrc;
            _ParseJson();
        }

        std::optional<std::string_view> TryGetUnparsedValue(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return it->second.value;
            else
                return std::nullopt;
        }

        std::optional<std::string> TryGetString(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return std::string(it->second.value.substr(1, it->second.value.length() - 2));
            else
                return std::nullopt;
        }

        std::optional<int> TryGetInt(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return std::stoi(std::string(it->second.value));
            else
                return std::nullopt;
        }

        std::optional<int64_t> TryGetLong(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return std::stoll(std::string(it->second.value));
            else
                return std::nullopt;
        }

        std::optional<float> TryGetFloat(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return std::stof(std::string(it->second.value));
            else
                return std::nullopt;
        }

        std::optional<double> TryGetDouble(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return std::stod(std::string(it->second.value));
            else
                return std::nullopt;
        }

        std::optional<size_t> TryGetBool(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return it->second.value == "true";
            else
                return std::nullopt;
        }

        std::optional<size_t> TryGetArraySize(const std::string& path)
        {
            auto it = _parsedNodes.find(path);
            if (it != _parsedNodes.end())
                return it->second.arraySize;
            else
                return std::nullopt;
        }

        std::string_view GetUnparsedValue(const std::string& path, std::string_view def = "")
        {
            return TryGetUnparsedValue(path).value_or(def);
        }

        std::string GetString(const std::string& path, const std::string& def = "")
        {
            return TryGetString(path).value_or(def);
        }

        int GetInt(const std::string& path, int def = 0)
        {
            return TryGetInt(path).value_or(def);
        }

        int64_t GetLong(const std::string& path, int64_t def = 0)
        {
            return TryGetLong(path).value_or(def);
        }

        float GetFloat(const std::string& path, float def = 0)
        {
            return TryGetFloat(path).value_or(def);
        }

        double GetDouble(const std::string& path, double def = 0)
        {
            return TryGetDouble(path).value_or(def);
        }

        bool GetBool(const std::string& path, bool def = false)
        {
            return TryGetBool(path).value_or(def);
        }

        size_t GetArraySize(const std::string& path, size_t def = 0)
        {
            return TryGetArraySize(path).value_or(def);
        }

        bool IsNull(const std::string& path)
        {
            auto val = TryGetUnparsedValue(path);
            if (!val)
                return false;
            return val.value() == "null";
        }

    private:
        struct _NodeView
        {
            std::string_view value;
            size_t arraySize;
            Type type;
        };

        std::string _jsonSrc;
        std::string_view _json;
        size_t _head;
        std::unordered_map<std::string, _NodeView> _parsedNodes;

        void _ParseJson()
        {
            _head = 0;
            _parsedNodes.clear();
            while (_head < _json.size())
            {
                char c = _json[_head];
                if (std::isspace(c)) { _head++; continue; }

                if (c == '[')
                {
                    _head++;
                    _ParseArray("");
                }
                else if (c == '{')
                {
                    _head++;
                    _ParseObject("");
                }
                else
                {
                    std::string_view val = _ParseValue();
                    _parsedNodes.insert({ "", { val, 0, Type::VALUE } });
                }
                break;
            }

            while (_head < _json.size())
            {
                char c = _json[_head];
                if (!std::isspace(c))
                    throw ParseException("Expected EOF, found " + c, _head);
                _head++;
            }
        }

        void _ParseArray(const std::string& path)
        {
            size_t start = _head;
            size_t size = 0;
            while (_head < _json.size())
            {
                char c = _json[_head];
                if (std::isspace(c)) { _head++; continue; }

                if (c == ']')
                {
                    _parsedNodes.insert({ path, { _json.substr(start, _head - start), size, Type::ARRAY}});
                    _head++;
                    return;
                }
                else if (c == '[')
                {
                    _head++;
                    _ParseArray(path + '[' + std::to_string(size++) + ']');
                }
                else if (c == '{')
                {
                    _head++;
                    _ParseObject(path + '[' + std::to_string(size++) + ']');
                }
                else
                {
                    std::string_view val = _ParseValue();
                    _parsedNodes.insert({ path + '[' + std::to_string(size++) + ']', { val, 0, Type::VALUE } });
                }
                _SkipCommaOrStopAt(']');
            }
            throw ParseException("Unexpected EOF", _head);
        }

        void _ParseObject(const std::string& path)
        {
            size_t start = _head;
            while (_head < _json.size())
            {
                char c = _json[_head];
                if (std::isspace(c)) { _head++; continue; }

                if (c == '}')
                {
                    _parsedNodes.insert({ path, { _json.substr(start, _head - start), 0, Type::OBJECT } });
                    _head++;
                    return;
                }
                else
                {
                    std::string_view key = _ReadString();
                    // Trim quotes
                    key = key.substr(1, key.length() - 2);
                    _SeekFromKeyToValue();

                    if (_json[_head] == '[')
                    {
                        _head++;
                        _ParseArray(path + '.' + std::string(key));
                    }
                    else if (_json[_head] == '{')
                    {
                        _head++;
                        _ParseObject(path + '.' + std::string(key));
                    }
                    else
                    {
                        std::string_view val = _ParseValue();
                        _parsedNodes.insert({ path + '.' + std::string(key), { val, 0, Type::VALUE } });
                    }
                    _SkipCommaOrStopAt('}');
                }
            }
            throw ParseException("Unexpected EOF", _head);
        }

        std::string_view _ParseValue()
        {
            std::string_view str;
            if (_json[_head] == 'n')
            {
                str = _json.substr(_head, 4);
                if (str != "null")
                    throw ParseException("Invalid value", _head);
                _head += 4;
            }
            else if (_json[_head] == 't')
            {
                str = _json.substr(_head, 4);
                if (str != "true")
                    throw ParseException("Invalid value", _head);
                _head += 4;
            }
            else if (_json[_head] == 'f')
            {
                str = _json.substr(_head, 5);
                if (str != "false")
                    throw ParseException("Invalid value", _head);
                _head += 5;
            }
            else if (_json[_head] == '"')
            {
                str = _ReadString();
            }
            else
            {
                str = _ReadNumberString();
            }
            return str;
        }

        std::string_view _ReadString()
        {
            if (_json[_head] != '"')
                throw ParseException("Expected \", got " + _json[_head], _head);

            size_t start = _head++;
            while (_head < _json.size())
            {
                if (_json[_head] == '"')
                    return _json.substr(start, ++_head - start);
                if (_json[_head] == '\\' && _head + 1 < _json.size() && _json[_head + 1] == '"')
                    _head++;
                _head++;
            }
            throw ParseException("EOF reached before end of string", _head);
        }

        std::string_view _ReadNumberString()
        {
            enum State
            {
                SIGN,
                FIRST_INT_DIGIT,
                INT_DIGITS,
                DEC_POINT,
                FIRST_DEC_DIGIT,
                DEC_DIGITS,
                EXP_SIGN,
                FIRST_EXP_DIGIT,
                EXP_DIGITS
            };

            size_t start = _head;
            int state = SIGN;
            while (_head < _json.size())
            {
                char c = _json[_head];
                if (state == SIGN)
                {
                    if (c == '-')
                        state = FIRST_INT_DIGIT;
                    else if (c == '0')
                        state = DEC_POINT;
                    else if (c >= '1' && c <= '9')
                        state = INT_DIGITS;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == FIRST_INT_DIGIT)
                {
                    if (c == '0')
                        state = DEC_POINT;
                    else if (c >= '1' && c <= '9')
                        state = INT_DIGITS;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == INT_DIGITS)
                {
                    if (c >= '0' && c <= '9')
                        state = INT_DIGITS;
                    else if (c == '.')
                        state = FIRST_DEC_DIGIT;
                    else if (_IsCharValidNumberEnd(c))
                        break;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == DEC_POINT)
                {
                    if (c == '.')
                        state = DEC_DIGITS;
                    else if (c == 'e' || c == 'E')
                        state = EXP_SIGN;
                    else if (_IsCharValidNumberEnd(c))
                        break;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == FIRST_DEC_DIGIT)
                {
                    if (c >= '0' && c <= '9')
                        state = DEC_DIGITS;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == DEC_DIGITS)
                {
                    if (c >= '0' && c <= '9')
                        state = DEC_DIGITS;
                    else if (c == 'e' || c == 'E')
                        state = EXP_SIGN;
                    else if (_IsCharValidNumberEnd(c))
                        break;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == EXP_SIGN)
                {
                    if (c == '+' || c == '-')
                        state = FIRST_EXP_DIGIT;
                    else if (c >= '0' && c <= '9')
                        state = EXP_DIGITS;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == FIRST_EXP_DIGIT)
                {
                    if (c >= '0' && c <= '9')
                        state = EXP_DIGITS;
                    else
                        throw ParseException("Invalid number", _head);
                }
                else if (state == EXP_DIGITS)
                {
                    if (c >= '0' && c <= '9')
                        state = EXP_DIGITS;
                    else if (_IsCharValidNumberEnd(c))
                        break;
                    else
                        throw ParseException("Invalid number", _head);
                }
                _head++;
            }
            if (state == SIGN || state == FIRST_INT_DIGIT || state == FIRST_DEC_DIGIT || state == EXP_SIGN || state == FIRST_EXP_DIGIT)
                throw ParseException("EOF reached before end of number", _head);

            return _json.substr(start, _head - start);
        }

        bool _IsCharValidNumberEnd(char c)
        {
            return std::isspace(c) || c == ',' || c == ']' || c == '}';
        }

        void _SeekFromKeyToValue()
        {
            bool colonFound = false;
            while (_head < _json.size())
            {
                if (_json[_head] == ':' && !colonFound)
                {
                    colonFound = true;
                }
                else if (!std::isspace(_json[_head]))
                {
                    if (colonFound)
                        return;
                    else
                        throw ParseException("Expected ':', got " + _json[_head], _head);
                }
                _head++;
            }
            throw ParseException("EOF reached before value", _head);
        }

        void _SkipCommaOrStopAt(char delimeter)
        {
            while (_head < _json.size())
            {
                if (_json[_head] == ',')
                {
                    _head++;
                    return;
                }
                else if (_json[_head] == delimeter)
                {
                    return;
                }
                else if (!std::isspace(_json[_head]))
                {
                    std::ostringstream ss("");
                    ss << "Expected ',' or '" << delimeter << "', got " << _json[_head];
                    throw ParseException(ss.str().c_str(), _head);
                }
                _head++;
            }
        }
    };
}