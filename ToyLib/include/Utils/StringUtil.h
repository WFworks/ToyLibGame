#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <algorithm>

namespace StringUtil
{
//==============================================================================
// 任意型 → std::string 変換
//------------------------------------------------------------------------------
// ・std::string → そのまま返す
// ・std::string に変換可能 → 変換
// ・それ以外 → std::to_string() を使う
//==============================================================================

template<typename T>
std::string ToString(const T& v)
{
    if constexpr (std::is_same_v<T, std::string>)
    {
        return v;
    }
    else if constexpr (std::is_convertible_v<T, std::string>)
    {
        return v;
    }
    else
    {
        return std::to_string(v);
    }
}

//==============================================================================
// 簡易フォーマット： "<<” を順番に置換
//------------------------------------------------------------------------------
// 例：Format("FPS: << / Pos: <<,<<", 60, x, y)
//      → "FPS: 60 / Pos: 12.3,45.6"
//
// ・引数の数だけ "<<” を前から置換する
// ・足りない "<<” は無視
// ・std::format ほど重くせず、軽くて安全
//==============================================================================

template<typename... Args>
std::string Format(const std::string& fmt, Args&&... args)
{
    // 可変引数を string へ変換して配列化
    std::vector<std::string> values = {
        ToString(std::forward<Args>(args))...
    };

    std::string result = fmt;
    size_t pos = 0;

    // "<<” を順に置換
    for (auto& val : values)
    {
        pos = result.find("<<", pos);
        if (pos == std::string::npos)
            break;

        result.replace(pos, 2, val);
        pos += val.size();
    }

    return result;
}


//==============================================================================
// Trim 系
//==============================================================================

// 左側の空白除去
inline std::string LTrim(const std::string& s)
{
    size_t pos = 0;
    while (pos < s.size() && std::isspace((unsigned char)s[pos])) pos++;
    return s.substr(pos);
}

// 右側の空白除去
inline std::string RTrim(const std::string& s)
{
    if (s.empty()) return s;
    size_t end = s.size() - 1;

    while (end > 0 && std::isspace((unsigned char)s[end])) end--;

    return s.substr(0, end + 1);
}

// 両側の空白除去
inline std::string Trim(const std::string& s)
{
    return RTrim(LTrim(s));
}


//==============================================================================
// Split / Join
//==============================================================================

// 区切り文字 split（空文字も拾う）
inline std::vector<std::string> Split(const std::string& s, char sep)
{
    std::vector<std::string> out;
    std::string current;

    for (char c : s)
    {
        if (c == sep)
        {
            out.emplace_back(current);
            current.clear();
        }
        else
        {
            current.push_back(c);
        }
    }
    out.emplace_back(current);
    return out;
}

// join
inline std::string Join(const std::vector<std::string>& arr, const std::string& sep)
{
    if (arr.empty()) return "";

    std::string out = arr[0];
    for (size_t i = 1; i < arr.size(); i++)
    {
        out += sep;
        out += arr[i];
    }
    return out;
}


//==============================================================================
// StartsWith / EndsWith
//==============================================================================

inline bool StartsWith(const std::string& s, const std::string& prefix)
{
    return s.size() >= prefix.size()
        && std::equal(prefix.begin(), prefix.end(), s.begin());
}

inline bool EndsWith(const std::string& s, const std::string& suffix)
{
    return s.size() >= suffix.size()
        && std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
}


//==============================================================================
// ToLower / ToUpper
//==============================================================================

inline std::string ToLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return out;
}

inline std::string ToUpper(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return out;
}


//==============================================================================
// ReplaceAll（部分文字列の一括置換）
//==============================================================================

inline std::string ReplaceAll(std::string s, const std::string& from, const std::string& to)
{
    if (from.empty()) return s;

    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos)
    {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}


//==============================================================================
// PadLeft / PadRight（固定長パディング）
//==============================================================================

inline std::string PadLeft(const std::string& s, size_t width, char fill = ' ')
{
    if (s.size() >= width) return s;
    return std::string(width - s.size(), fill) + s;
}

inline std::string PadRight(const std::string& s, size_t width, char fill = ' ')
{
    if (s.size() >= width) return s;
    return s + std::string(width - s.size(), fill);
}

} // namespace StringUtil


