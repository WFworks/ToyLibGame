// Audio/SoundMixer.h
#pragma once

#include <string>
#include <memory>
#include <vector>

#if defined(__APPLE__)
    #include <AL/al.h>      // openal-soft (Homebrew) 想定
    #include <AL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

namespace toy {

class AssetManager;
class Music;
class SoundEffect;

class SoundMixer
{
public:
    SoundMixer(class AssetManager* assetManager);
    ~SoundMixer();

    // 毎フレーム呼ぶ（BGMストリーミング＋ワンショットSEのクリーンアップ）
    void Update(float deltaTime);

    void SetBGMEnable(bool enable);
    void SetSoundEnable(bool enable);
    void SetVolume(float volume); // 0.0〜1.0

    bool LoadBGM(const std::string& fileName);
    void PlayBGM();   // ループ再生
    void StopBGM();

    // 2D効果音（ワンショット）
    void PlaySoundEffect(const std::string& fileName);

private:
    AssetManager* mAssetManager = nullptr;

    bool  mBgmEnabled  = true;
    bool  mSoundEnabled = true;
    float mVolume      = 0.3f;

    // OpenAL デバイス/コンテキスト
    ALCdevice*  mDevice  = nullptr;
    ALCcontext* mContext = nullptr;

    // BGM 用
    static constexpr int BGM_NUM_BUFFERS = 4;
    static constexpr int BGM_CHUNK_SIZE  = 4096 * 4;

    std::shared_ptr<Music> mCurrentBGM;
    ALuint  mBgmSource                          = 0;
    ALuint  mBgmBuffers[BGM_NUM_BUFFERS]        = {};
    bool    mBgmPlaying                         = false;
    bool    mBgmLoop                            = true; // PlayBGM はループ扱い
    std::vector<unsigned char> mBgmDecodeBuffer;

    // ワンショットSE用ソース
    std::vector<ALuint> mOneShotSources;

private:
    void InitOpenAL();
    void ShutdownOpenAL();

    void InitBGMSource();
    void ShutdownBGMSource();
};

} // namespace toy

/*
#pragma once
#include <string>
#include <memory>

namespace toy {

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

} // namespace toy

*/
