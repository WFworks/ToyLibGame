// Audio/SoundMixer.h
#pragma once

#include <string>
#include <vector>
#include <memory>

#include <AL/al.h>
#include <AL/alc.h>

#include "Utils/MathUtil.h"      // Matrix4 / Vector3

namespace toy {

class AssetManager;
class Music;
class SoundEffect;

class SoundMixer
{
public:
    SoundMixer(AssetManager* assetManager);
    ~SoundMixer();

    void SetBGMEnable(bool enable);
    void SetSoundEnable(bool enable);
    void SetVolume(float volume);

    bool LoadBGM(const std::string& fileName);
    void PlayBGM();
    void StopBGM();

    void PlaySoundEffect(const std::string& fileName);

    // Renderer から invViewMatrix を渡す
    void Update(float deltaTime, const Matrix4& invViewMatrix);

private:
    // OpenAL 基本
    void InitOpenAL();
    void ShutdownOpenAL();

    // BGM
    void InitBGMSource();
    void ShutdownBGMSource();

private:
    AssetManager* mAssetManager;

    // OpenAL デバイス/コンテキスト
    ALCdevice*  mDevice;
    ALCcontext* mContext;

    // SE（ワンショット）
    std::vector<ALuint> mOneShotSources;

    // BGM ストリーミング
    static constexpr int BGM_NUM_BUFFERS = 4;
    static constexpr int BGM_CHUNK_SIZE  = 32 * 1024;

    ALuint mBgmSource;
    ALuint mBgmBuffers[BGM_NUM_BUFFERS];

    bool  mBgmPlaying;
    bool  mBgmLoop;

    std::shared_ptr<Music> mCurrentBGM;
    std::vector<unsigned char> mBgmDecodeBuffer;   // ← 修正点：unsigned char

    // 共通設定
    bool  mBgmEnabled;
    bool  mSoundEnabled;
    float mVolume;
};

} // namespace toy
