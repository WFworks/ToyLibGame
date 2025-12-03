#include "Asset/AssetManager.h"
#include "Asset/Material/Texture.h"
#include "Asset/Geometry/Mesh.h"
#include "Asset/Audio/SoundEffect.h"
#include "Asset/Audio/Music.h"
#include "Asset/Font/TextFont.h"

#include <iostream>

AssetManager::AssetManager()
: mAssetsPath("GameApp/Assets") // デフォルト値
{
}

void AssetManager::UnloadData()
{
    // テクスチャ削除
    mTextures.clear();
    // メッシュ削除
    mMeshes.clear();
    // サウンド削除
    mSoundEffects.clear();
    // BGM削除
    mMusics.clear();
    // フォント削除
    mTextFonts.clear();
}

// テクスチャ取り出し
std::shared_ptr<Texture> AssetManager::GetTexture(const std::string &fileName)
{
    auto iter = mTextures.find(fileName);
    if (iter != mTextures.end())
    {
        return iter->second; // すでにあるのでそれを返す
    }
    else
    {
        auto tex = std::make_shared<Texture>();
        if (tex->Load(fileName, this))
        {
            mTextures[fileName] = tex;
            return tex;
        }
    }
    return nullptr; // 失敗したら null
}

// 埋め込みテクスチャ
std::shared_ptr<Texture> AssetManager::GetEmbeddedTexture(const std::string& nameKey, const uint8_t* data, size_t dataSize)
{
    auto iter = mTextures.find(nameKey);
    if (iter != mTextures.end())
    {
        return iter->second;
    }

    auto tex = std::make_shared<Texture>();
    if (tex->LoadFromMemory(data, static_cast<unsigned int>(dataSize)))
    {
        mTextures[nameKey] = tex;
        return tex;
    }

    return nullptr;
}

// メッシュ取り出し
std::shared_ptr<Mesh> AssetManager::GetMesh(const std::string& fileName, bool isRightHanded)
{
    auto iter = mMeshes.find(fileName);
    if (iter != mMeshes.end())
    {
        return iter->second;
    }

    auto mesh = std::make_shared<Mesh>();
 
    if (mesh->Load(fileName, this, isRightHanded))
    {
        mMeshes[fileName] = mesh;
        return mesh;
    }

    return nullptr;
}

// 効果音を取得
std::shared_ptr<SoundEffect> AssetManager::GetSoundEffect(const std::string& fileName)
{
    auto iter = mSoundEffects.find(fileName);
    if (iter != mSoundEffects.end())
    {
        return iter->second;
    }

    auto se = std::make_shared<SoundEffect>();
    if (se->Load(fileName, this))
    {
        mSoundEffects[fileName] = se;
        return se;
    }
    return nullptr;
}

// BGMを取得
std::shared_ptr<Music> AssetManager::GetMusic(const std::string& fileName)
{
    auto iter = mMusics.find(fileName);
    if (iter != mMusics.end())
    {
        return iter->second;
    }

    auto music = std::make_shared<Music>();
    if (music->Load(fileName, this))
    {
        mMusics[fileName] = music;
        return music;
    }
    return nullptr;
}

// フォント
std::shared_ptr<TextFont> AssetManager::GetFont(const std::string& fileName, int pointSize)
{
    // フォントはサイズ違いもあるので key にサイズ情報を含める
    const std::string key = fileName + "#" + std::to_string(pointSize);

    // 既にロード済みならそれを返す
    auto iter = mTextFonts.find(key);
    if (iter != mTextFonts.end())
    {
        return iter->second;
    }

    // 新規ロード
    auto font = std::make_shared<TextFont>();

    // Texture::Load と同じく AssetsPath を前につける
    // GetTexture の実装と同じ感覚で：
    //   fullPath = mAssetsPath + fileName;
    // としておく（fileName 側で "/Fonts/xxx.ttf" などを渡す想定）
    const std::string fullPath = mAssetsPath + fileName;
    if (!font->Load(fullPath, pointSize))
    {
        std::cerr << "[AssetManager] Failed to load font: "
                  << fullPath << " (size: " << pointSize << ")"
                  << std::endl;
        return nullptr;
    }

    mTextFonts.emplace(key, font);
    return font;
}
