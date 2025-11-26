#include "Texture.h"
#include "AssetManager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <vector>
#include <algorithm>

Texture::Texture()
: mTextureID(0)
, mWidth(0)
, mHeight(0)
{
	
}

Texture::~Texture()
{
	
}


bool Texture::Load(const std::string& fileName, AssetManager* assetManager)
{
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF) == 0)
    {
        SDL_Log("Failed to initialize SDL2_image: %s", IMG_GetError());
        return false;
    }
    
    std::string fullName = assetManager->GetAssetsPath() + fileName;
    SDL_Surface* image = IMG_Load(fullName.c_str());
    if (!image)
    {
        SDL_Log("Failed to load image %s: %s", fileName.c_str(), IMG_GetError());
        return false;
    }

    SDL_Surface* tmpImage = SDL_CreateRGBSurface(
        0, image->w, image->h, 32,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
    );
    if (!tmpImage)
    {
        SDL_Log("Failed to create temporary surface: %s", SDL_GetError());
        SDL_FreeSurface(image);
        return false;
    }

    SDL_Surface* convertedImage = SDL_ConvertSurface(image, tmpImage->format, 0);
    SDL_FreeSurface(image);
    SDL_FreeSurface(tmpImage);

    if (!convertedImage)
    {
        SDL_Log("Failed to convert surface: %s", SDL_GetError());
        return false;
    }

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8, convertedImage->w, convertedImage->h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, convertedImage->pixels
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ← ★追加すべき！
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // ← ★追加すべき！

    mWidth = convertedImage->w;
    mHeight = convertedImage->h;

    SDL_FreeSurface(convertedImage);
    return true;
}

bool Texture::LoadFromMemory(const void* data, int size)
{
    SDL_RWops* rw = SDL_RWFromConstMem(data, size);
    if (!rw)
    {
        SDL_Log("Failed to create RWops from memory: %s", SDL_GetError());
        return false;
    }

    SDL_Surface* image = IMG_Load_RW(rw, 1); // 1: SDLにRWopsも解放させる
    if (!image)
    {
        SDL_Log("Failed to load image from memory: %s", IMG_GetError());
        return false;
    }

    // OpenGL テクスチャ作成
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        image->w, image->h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image->pixels
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    mWidth = image->w;
    mHeight = image->h;

    SDL_FreeSurface(image);
    return true;
}
/*
bool Texture::LoadFromMemory(const void* data, int width, int height)
{
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    mWidth = width;
    mHeight = height;

    return true;
}
*/

bool Texture::LoadFromMemory(const void* data, int width, int height)
{
    if (mTextureID != 0)
    {
        glDeleteTextures(1, &mTextureID);
    }

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,  // ← 内部フォーマットを明示
        width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // ← 追加
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // ← 追加

    mWidth = width;
    mHeight = height;

    return true;
}
// レンダリング用テクスチャを生成
void Texture::CreateForRendering(int w, int h, unsigned int format)
{
    mWidth = w;
    mHeight = h;
    // テクスチャ生成
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    // 空のテクスチャ
    glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


// テクスチャ削除
void Texture::Unload()
{
    glActiveTexture(GL_TEXTURE0); 
	glDeleteTextures(1, &mTextureID);
}

// アクティブ化（Bind）
void Texture::SetActive(int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, mTextureID);
}

void Texture::CreateShadowMap(int width, int height)
{
    mWidth = width;
    mHeight = height;

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    // 内部フォーマットとデータ型
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, // ★明示的な内部フォーマット
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    
    // フィルター（まずはNEARESTで明示的に挙動を確認）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // ラップモード
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // シャドウマッピングに必須の比較設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

bool Texture::CreateAlphaCircle(int size, float centerX, float centerY, Vector3 color, float blendPow)
{
    
    if (size <= 0) return false;

    std::vector<uint8_t> pixels(size * size * 4); // RGBA

    // 中心座標をピクセルに変換
    float cx = centerX * size;
    float cy = centerY * size;

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy) / (size / 3.0f); // 正規化
            float alpha = 1.0f - std::pow(std::clamp(dist, 0.0f, 1.0f), blendPow);

            int index = (y * size + x) * 4;
            pixels[index + 0] = static_cast<int>(255.f * color.x);
            pixels[index + 1] = static_cast<int>(255.f * color.y);
            pixels[index + 2] = static_cast<int>(255.f * color.z);
            pixels[index + 3] = static_cast<uint8_t>(alpha * 255);
        }
    }

    // OpenGL テクスチャ作成
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    mWidth = size;
    mHeight = size;

    return true;
}

bool Texture::CreateRadialRays(int size, int numRays, float fadePow, float rayStrength, float intensityScale)
{
    if (size <= 0 || numRays <= 0) return false;

    std::vector<uint8_t> pixels(size * size * 4);

    float cx = size * 0.5f;
    float cy = size * 0.5f;
    float maxDist = size * 0.5f;

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy) / maxDist;

            float angle = std::atan2(dy, dx);
            float ray = std::abs(std::sin(angle * numRays)); // 光条の強度

            float alpha = (1.0f - std::clamp(dist, 0.0f, 1.0f));
            alpha = std::pow(alpha, fadePow) * ray * rayStrength;

            alpha = std::clamp(alpha * intensityScale, 0.0f, 1.0f); // 最終的に暗めに調整

            int index = (y * size + x) * 4;
            pixels[index + 0] = static_cast<uint8_t>(alpha * 255);
            pixels[index + 1] = static_cast<uint8_t>(alpha * 255);
            pixels[index + 2] = static_cast<uint8_t>(alpha * 200); // 黄味強め
            pixels[index + 3] = static_cast<uint8_t>(alpha * 255); // Aも調整（加算なら影響しないけど）
        }
    }

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    mWidth = size;
    mHeight = size;

    return true;
}
