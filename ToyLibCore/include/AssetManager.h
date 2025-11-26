#pragma once
#include <unordered_map>
#include <memory>
#include <string>

class AssetManager
{
public:
    AssetManager();
    
    // メッシュ登録
    std::shared_ptr<class Mesh> GetMesh(const std::string& fileName, bool isRightHanded = false);
    // テクスチャ登録
    std::shared_ptr<class Texture> GetTexture(const std::string& fileName);
    std::shared_ptr<class Texture> GetEmbeddedTexture(const std::string& nameKey, const uint8_t* data, size_t dataSize);
    
    // SE
    std::shared_ptr<class SoundEffect> GetSoundEffect(const std::string& fileName);
    // BGM
    std::shared_ptr<class Music> GetMusic(const std::string& fileName);
    
    std::string GetAssetsPath() const { return mAssetsPath; };
    
    void UnloadData();
    
    void SetAssetsPath(const std::string &path) { mAssetsPath = path; };
private:
    std::unordered_map<std::string, std::shared_ptr<class Texture>> mTextures;
    std::unordered_map<std::string, std::shared_ptr<class Mesh>> mMeshes;
    std::unordered_map<std::string, std::shared_ptr<class SoundEffect>> mSoundEffects;
    std::unordered_map<std::string, std::shared_ptr<class Music>> mMusics;
    
    std::string mAssetsPath;
};
