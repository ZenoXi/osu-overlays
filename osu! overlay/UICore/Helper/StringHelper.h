#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <string>
#include <vector>

std::string wstring_to_string(const std::wstring& ws);
std::wstring string_to_wstring(const std::string& s);
std::string wstr_to_utf8(const std::wstring& ws);
std::wstring utf8_to_wstr(const std::string& s);
void split_str(const std::string& str, std::vector<std::string>& output, char split = ' ', bool ignoreEmpty = false);
void split_wstr(const std::wstring& str, std::vector<std::wstring>& output, wchar_t split = L' ', bool ignoreEmpty = false);
template<size_t Count>
void split_str(const std::string& str, std::array<std::string, Count>& output, char split = ' ', bool ignoreEmpty = false);
template<size_t Count>
void split_wstr(const std::wstring& str, std::array<std::wstring, Count>& output, wchar_t split = L' ', bool ignoreEmpty = false);
int64_t str_to_int(const std::string& str, int base = 10);
float str_to_float(const std::string& str);
double str_to_double(const std::string& str);
std::string int_to_str(int64_t num);
std::string float_to_str(float num);
std::string double_to_str(double num);
std::string to_uppercase(const std::string& str);
std::string to_lowercase(const std::string& str);
int find_text_in_vec(const std::string& str, const std::vector<std::string>& vec);
int find_text_in_arr(const std::string& str, std::string arr[], int size);
std::string extract_str_until(const std::string& str, char tc = ' ');
void erase_str_until(std::string& str, char tc = ' ');
std::string exer_str_until(std::string& str, char tc = ' ');

template<size_t Count>
void split_str(const std::string& str, std::array<std::string, Count>& output, char split, bool ignoreEmpty)
{
    int beginIndex = 0;
    for (int count = 0; count < output.size(); count++)
    {
        // Find start point
        if (ignoreEmpty)
        {
            while (str[beginIndex] == split)
                beginIndex++;
        }
        if (count == output.size() - 1)
            break;

        for (int i = beginIndex; i < str.length(); i++)
        {
            if (str[i] == split)
            {
                output[count] = str.substr(beginIndex, i - beginIndex);
                beginIndex = i + 1;
                break;
            }
        }
    }
    output[output.size() - 1] = str.substr(beginIndex);
}

template<size_t Count>
void split_wstr(const std::wstring& str, std::array<std::wstring, Count>& output, wchar_t split, bool ignoreEmpty)
{
    int beginIndex = 0;
    for (int count = 0; count < output.size(); count++)
    {
        // Find start point
        if (ignoreEmpty)
        {
            while (str[beginIndex] == split)
                beginIndex++;
        }
        if (count == output.size() - 1)
            break;

        for (int i = beginIndex; i < str.length(); i++)
        {
            if (str[i] == split)
            {
                output[count] = str.substr(beginIndex, i - beginIndex);
                beginIndex = i + 1;
                break;
            }
        }
    }
    output[output.size() - 1] = str.substr(beginIndex);
}