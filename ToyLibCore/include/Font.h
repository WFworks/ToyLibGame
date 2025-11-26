#pragma once

#include <string>

class Font
{
public:
    Font();
    ~Font();
    
    // フォント読み込み
    bool Load(const std::string& fileName);
};
