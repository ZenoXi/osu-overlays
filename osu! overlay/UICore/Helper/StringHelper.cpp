#include "StringHelper.h"

#include <sstream>
#include <codecvt>

// Use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
std::string wstring_to_string(const std::wstring& ws)
{
    // Setup converter
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.to_bytes(ws);
}

std::wstring string_to_wstring(const std::string& s)
{
    // Setup converter
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.from_bytes(s);
}

std::string wstr_to_utf8(const std::wstring& ws)
{
    // Setup converter
    using convert_type = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.to_bytes(ws);
}

std::wstring utf8_to_wstr(const std::string& s)
{
    // Setup converter
    using convert_type = std::codecvt_utf8_utf16<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    return converter.from_bytes(s);
}

void split_str(const std::string& str, std::vector<std::string>& output, char split, bool ignoreEmpty)
{
    int beginIndex = 0;
    while (beginIndex < str.length())
    {
        // Find start point
        if (ignoreEmpty)
        {
            while (str[beginIndex] == split)
                beginIndex++;
        }
        if (beginIndex == str.length())
            break;

        int i = beginIndex;
        for (; i < str.length(); i++)
        {
            if (str[i] == split)
            {
                output.push_back(str.substr(beginIndex, i - beginIndex));
                beginIndex = i + 1;
                break;
            }
        }
        if (i == str.length())
        {
            output.push_back(str.substr(beginIndex));
            break;
        }
    }
}

void split_wstr(const std::wstring& str, std::vector<std::wstring>& output, wchar_t split, bool ignoreEmpty)
{
    int beginIndex = 0;
    while (beginIndex < str.length())
    {
        // Find start point
        if (ignoreEmpty)
        {
            while (str[beginIndex] == split)
                beginIndex++;
        }
        if (beginIndex == str.length())
            break;

        int i = beginIndex;
        for (; i < str.length(); i++)
        {
            if (str[i] == split)
            {
                output.push_back(str.substr(beginIndex, i - beginIndex));
                beginIndex = i + 1;
                break;
            }
        }
        if (i == str.length())
        {
            output.push_back(str.substr(beginIndex));
            break;
        }
    }
}

int64_t str_to_int(const std::string& str, int base)
{
    bool negative = false;

    std::vector<int> nums;
    nums.reserve(19);

    // Get numbers from string
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '-') {
            negative = true;
        }
        else {
            int val = str[i] - 48;

            if (val >= 0 && val <= 9) {
                nums.push_back(val);
            }
        }

        if (nums.size() >= 19)
            break;
    }
    if (nums.size() == 0) return 0;

    // Combine digits into a number
    size_t size = nums.size();
    int64_t mult = 1;
    int64_t fnum = 0;

    for (int i = size - 1; i >= 0; i--) {
        fnum += nums[i] * mult;
        mult *= base;
    }

    if (negative) fnum = -fnum;

    return fnum;
}

float str_to_float(const std::string& str)
{
    return std::stof(str);
}

double str_to_double(const std::string& str)
{
    return std::stod(str);
}

std::string int_to_str(int64_t num)
{
    std::ostringstream ss;
    ss << num;
    return ss.str();
}

std::string float_to_str(float num)
{
    std::ostringstream ss("");
    ss << num;
    return ss.str();
}

std::string double_to_str(double num)
{
    std::ostringstream ss("");
    ss << num;
    return ss.str();
}

std::string to_uppercase(const std::string& str)
{
    std::string newStr = str;
    for (int i = 0; i < newStr.size(); i++)
    {
        newStr[i] = toupper(newStr[i]);
    }
    return newStr;
}

std::string to_lowercase(const std::string& str)
{
    std::string newStr = str;
    for (int i = 0; i < newStr.size(); i++)
    {
        newStr[i] = tolower(newStr[i]);
    }
    return newStr;
}

int find_text_in_vec(const std::string& str, std::vector<std::string> vec)
{
    for (int i = 0; i < vec.size(); i++)
        if (vec[i] == str)
            return i;

    return -1;
}

int find_text_in_arr(const std::string& str, std::string arr[], int size)
{
    for (int i = 0; i < size; i++)
        if (arr[i] == str)
            return i;

    return -1;
}

std::string extract_str_until(const std::string& str, char tc)
{
    std::string s;
    s.reserve(10);

    for (unsigned i = 0; i < str.length(); i++) {
        if (str[i] != tc)
            s.push_back(str[i]);
        else
            break;
    }

    return s;
}

void erase_str_until(std::string& str, char tc)
{
    int l = extract_str_until(str, tc).length();

    str.erase(str.begin(), str.begin() + l);
    while (str[0] == tc) {
        str.erase(str[0]);
    }
}

std::string exer_str_until(std::string& str, char tc)
{
    std::string s = extract_str_until(str, tc);

    str.erase(str.begin(), str.begin() + s.length());
    while (str[0] == tc) {
        str.erase(str.begin(), str.begin() + 1);
    }

    return s;
}