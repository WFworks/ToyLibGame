#pragma once
#include <string>
#include <memory>

class SoundMixer
{
public:
    SoundMixer(class AssetManager* assetManager);
    ~SoundMixer();

    void SetBGMEnable(bool enable);
    void SetSoundEnable(bool enable);
    void SetVolume(float volume);

    bool LoadBGM(const std::string& fileName);
    void PlayBGM();
    void StopBGM();

    void PlaySoundEffect(const std::string& fileName);

private:
    AssetManager* mAssetManager;
    std::shared_ptr<class Music> mCurrentBGM;

    bool mBgmEnabled;
    bool mSoundEnabled;
    float mVolume;
};
