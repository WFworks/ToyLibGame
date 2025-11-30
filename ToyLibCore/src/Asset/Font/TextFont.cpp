#include "Asset/Font/TextFont.h"

#include <iostream>

TextFont::TextFont()
{
}

TextFont::~TextFont()
{
    Unload();
}

bool TextFont::Load(const std::string& filePath, int pointSize)
{
    // すでにロード済みなら一度解放
    if (mFont)
    {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }

    mFont = TTF_OpenFont(filePath.c_str(), pointSize);
    if (!mFont)
    {
        std::cerr << "TTF_OpenFont failed: " << TTF_GetError()
                  << " (file: " << filePath << ", size: " << pointSize << ")"
                  << std::endl;
        return false;
    }

    mFilePath  = filePath;
    mPointSize = pointSize;
    return true;
}

void TextFont::Unload()
{
    if (mFont)
    {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }
}
