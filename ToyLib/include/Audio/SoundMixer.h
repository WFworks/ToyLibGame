// Audio/SoundMixer.h
#pragma once

#include <string>
#include <memory>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>

const int BGM_NUM_BUFFERS = 4;
const int BGM_CHUNK_SIZE  = 4096 * 4;

namespace toy {

class AssetManager;
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
    class AssetManager* mAssetManager = nullptr;

    bool  mBgmEnabled;
    bool  mSoundEnabled;
    float mVolume;

    // OpenAL デバイス/コンテキスト
    ALCdevice*  mDevice;
    ALCcontext* mContext;


    std::shared_ptr<class Music> mCurrentBGM;
    ALuint  mBgmSource;
    ALuint  mBgmBuffers[BGM_NUM_BUFFERS] = {};
    bool    mBgmPlaying;
    bool    mBgmLoop;
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

