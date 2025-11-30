#pragma once

#include <string>
#include <SDL2/SDL_ttf.h>

class TextFont
{
public:
    TextFont();
    ~TextFont();

    // フォントファイルを読み込む
    // 例: Load("GameApp/Assets/Fonts/NotoSansJP-Regular.ttf", 24);
    bool Load(const std::string& filePath, int pointSize);

    // 明示的に解放したいとき
    void Unload();

    // 有効なフォントかどうか
    bool IsValid() const { return mFont != nullptr; }

    // SDL_ttf の生ポインタ（Renderer などから使用）
    TTF_Font* GetNativeFont() const { return mFont; }

    // 情報系
    const std::string& GetFilePath() const { return mFilePath; }
    int GetPointSize() const { return mPointSize; }

private:
    // コピー禁止（AssetManager で shared_ptr 管理前提）
    TextFont(const TextFont&) = delete;
    TextFont& operator=(const TextFont&) = delete;

    TTF_Font*   mFont       = nullptr;
    std::string mFilePath   {};
    int         mPointSize  = 0;
};
