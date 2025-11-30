#pragma once
#include <string>
#include <vector>
#include <type_traits>

namespace StringUtil
{
    // 任意型 → string
    template<typename T>
    std::string ToString(const T& v)
    {
        if constexpr (std::is_same_v<T, std::string>)
            return v;
        else if constexpr (std::is_convertible_v<T, std::string>)
            return v;
        else
            return std::to_string(v);
    }

    // "<<“ 置換フォーマット
    template<typename... Args>
    std::string Format(const std::string& fmt, Args&&... args)
    {
        std::vector<std::string> values = {
            ToString(std::forward<Args>(args))...
        };

        std::string result = fmt;
        size_t pos = 0;
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
}
