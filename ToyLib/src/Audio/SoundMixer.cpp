// Audio/SoundMixer.cpp
#include "Audio/SoundMixer.h"

#include "Asset/AssetManager.h"
#include "Asset/Audio/Music.h"
#include "Asset/Audio/SoundEffect.h"

namespace toy {

SoundMixer::SoundMixer(AssetManager* assetManager)
: mAssetManager(assetManager)
, mDevice(nullptr)
, mContext(nullptr)
, mBgmEnabled(true)
, mSoundEnabled(true)
, mVolume(0.3f)
, mBgmSource(0)
, mBgmPlaying(false)
, mBgmLoop(true) // PlayBGM はループ扱い
{
    InitOpenAL();
    mBgmDecodeBuffer.resize(BGM_CHUNK_SIZE);
}

SoundMixer::~SoundMixer()
{
    ShutdownBGMSource();

    // 残っているワンショットソースを削除
    for (ALuint src : mOneShotSources)
    {
        alSourceStop(src);
        alDeleteSources(1, &src);
    }
    mOneShotSources.clear();

    ShutdownOpenAL();
}

void SoundMixer::InitOpenAL()
{
    mDevice = alcOpenDevice(nullptr);
    if (!mDevice)
    {
        std::printf("[SoundMixer] Failed to open OpenAL device\n");
        return;
    }

    mContext = alcCreateContext(mDevice, nullptr);
    if (!mContext)
    {
        std::printf("[SoundMixer] Failed to create OpenAL context\n");
        alcCloseDevice(mDevice);
        mDevice = nullptr;
        return;
    }

    if (!alcMakeContextCurrent(mContext))
    {
        std::printf("[SoundMixer] Failed to make OpenAL context current\n");
        alcDestroyContext(mContext);
        mContext = nullptr;
        alcCloseDevice(mDevice);
        mDevice = nullptr;
        return;
    }

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        std::printf("[SoundMixer] OpenAL error after init: 0x%x\n", err);
    }
}

void SoundMixer::ShutdownOpenAL()
{
    if (mContext)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(mContext);
        mContext = nullptr;
    }
    if (mDevice)
    {
        alcCloseDevice(mDevice);
        mDevice = nullptr;
    }
}

void SoundMixer::InitBGMSource()
{
    if (mBgmSource == 0)
    {
        alGenSources(1, &mBgmSource);
        alGenBuffers(BGM_NUM_BUFFERS, mBgmBuffers);

        alSource3f(mBgmSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSourcef (mBgmSource, AL_ROLLOFF_FACTOR, 0.0f);
        alSourcei (mBgmSource, AL_LOOPING, AL_FALSE); // ループは自前制御
    }
}

void SoundMixer::ShutdownBGMSource()
{
    if (mBgmSource != 0)
    {
        alSourceStop(mBgmSource);

        ALint queued = 0;
        alGetSourcei(mBgmSource, AL_BUFFERS_QUEUED, &queued);
        while (queued-- > 0)
        {
            ALuint buf = 0;
            alSourceUnqueueBuffers(mBgmSource, 1, &buf);
        }

        alDeleteSources(1, &mBgmSource);
        alDeleteBuffers(BGM_NUM_BUFFERS, mBgmBuffers);
        mBgmSource = 0;
    }
    mBgmPlaying = false;
}

void SoundMixer::SetBGMEnable(bool enable)
{
    mBgmEnabled = enable;
    if (!mBgmEnabled)
    {
        StopBGM();
    }
}

void SoundMixer::SetSoundEnable(bool enable)
{
    mSoundEnabled = enable;
}

void SoundMixer::SetVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    mVolume = volume;

    // BGM に反映
    if (mBgmSource != 0)
    {
        alSourcef(mBgmSource, AL_GAIN, mVolume);
    }
}

bool SoundMixer::LoadBGM(const std::string& fileName)
{
    mCurrentBGM = mAssetManager->GetMusic(fileName);
    return (mCurrentBGM != nullptr);
}

void SoundMixer::PlayBGM()
{
    if (!mBgmEnabled || !mCurrentBGM) return;

    
    InitBGMSource();

    // 既存キューをクリア
    alSourceStop(mBgmSource);
    ALint queued = 0;
    alGetSourcei(mBgmSource, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0)
    {
        ALuint buf = 0;
        alSourceUnqueueBuffers(mBgmSource, 1, &buf);
    }

    // 再生位置リセット
    mCurrentBGM->Rewind();

    // 最初の複数バッファをデコードしてキューへ
    for (int i = 0; i < BGM_NUM_BUFFERS; ++i)
    {
        size_t bytes = mCurrentBGM->ReadChunk(mBgmDecodeBuffer.data(), BGM_CHUNK_SIZE);
        if (bytes == 0) break;

        ALenum format = (mCurrentBGM->GetChannels() == 2)
            ? AL_FORMAT_STEREO16
            : AL_FORMAT_MONO16;

        alBufferData(mBgmBuffers[i],
                     format,
                     mBgmDecodeBuffer.data(),
                     static_cast<ALsizei>(bytes),
                     static_cast<ALsizei>(mCurrentBGM->GetRate()));

        alSourceQueueBuffers(mBgmSource, 1, &mBgmBuffers[i]);
    }

    alSourcef(mBgmSource, AL_GAIN, mVolume);
    alSourcePlay(mBgmSource);
    mBgmPlaying = true;
    mBgmLoop    = true; // 旧APIと同じくループ前提
}

void SoundMixer::StopBGM()
{
    if (mBgmSource != 0)
    {
        alSourceStop(mBgmSource);
        mBgmPlaying = false;
        if (mCurrentBGM)
        {
            mCurrentBGM->Rewind();
        }
    }
}
/*
void SoundMixer::PlaySoundEffect(const std::string& fileName)
{
    if (!mSoundEnabled) return;

    auto se = mAssetManager->GetSoundEffect(fileName);
    if (!se) return;

    ALuint src = 0;
    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, se->GetBuffer());
    alSourcef(src, AL_GAIN, mVolume); // マスター音量に合わせる
    alSource3f(src, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcef(src, AL_ROLLOFF_FACTOR, 0.0f);

    alSourcePlay(src);
    mOneShotSources.push_back(src);
}
*/
void SoundMixer::PlaySoundEffect(const std::string& fileName)
{
    if (!mSoundEnabled) return;
    printf("play SE called \n");
    auto se = mAssetManager->GetSoundEffect(fileName);
    if (!se)
    {
        printf("[SoundMixer] SE load failed: %s\n", fileName.c_str());
        return;
    }

    ALuint src = 0;
    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, se->GetBuffer());
    alSourcef(src, AL_GAIN, mVolume);
    alSource3f(src, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcef(src, AL_ROLLOFF_FACTOR, 0.0f);

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        printf("[SoundMixer] SE AL error before play: 0x%x\n", err);
    }

    alSourcePlay(src);

    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        printf("[SoundMixer] SE AL error after play: 0x%x\n", err);
    }

    mOneShotSources.push_back(src);
}


void SoundMixer::Update(float)
{
    // BGM ストリーミング
    if (mBgmPlaying && mCurrentBGM && mBgmSource != 0)
    {
        ALint processed = 0;
        alGetSourcei(mBgmSource, AL_BUFFERS_PROCESSED, &processed);

        while (processed-- > 0)
        {
            ALuint buf = 0;
            alSourceUnqueueBuffers(mBgmSource, 1, &buf);

            size_t bytes = mCurrentBGM->ReadChunk(mBgmDecodeBuffer.data(), BGM_CHUNK_SIZE);

            if (bytes > 0)
            {
                ALenum format = (mCurrentBGM->GetChannels() == 2)
                    ? AL_FORMAT_STEREO16
                    : AL_FORMAT_MONO16;

                alBufferData(buf,
                             format,
                             mBgmDecodeBuffer.data(),
                             static_cast<ALsizei>(bytes),
                             static_cast<ALsizei>(mCurrentBGM->GetRate()));

                alSourceQueueBuffers(mBgmSource, 1, &buf);
            }
            else
            {
                if (mBgmLoop)
                {
                    mCurrentBGM->Rewind();
                    bytes = mCurrentBGM->ReadChunk(mBgmDecodeBuffer.data(), BGM_CHUNK_SIZE);

                    if (bytes > 0)
                    {
                        ALenum format = (mCurrentBGM->GetChannels() == 2)
                            ? AL_FORMAT_STEREO16
                            : AL_FORMAT_MONO16;

                        alBufferData(buf,
                                     format,
                                     mBgmDecodeBuffer.data(),
                                     static_cast<ALsizei>(bytes),
                                     static_cast<ALsizei>(mCurrentBGM->GetRate()));

                        alSourceQueueBuffers(mBgmSource, 1, &buf);
                    }
                }
            }
        }
    }

    // ワンショットSEの後片付け
    std::vector<ALuint> stillAlive;
    stillAlive.reserve(mOneShotSources.size());

    for (ALuint src : mOneShotSources)
    {
        ALint state = 0;
        alGetSourcei(src, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED)
        {
            alDeleteSources(1, &src);
        }
        else
        {
            stillAlive.push_back(src);
        }
    }

    mOneShotSources.swap(stillAlive);
}

} // namespace toy

/*
#include "Audio/SoundMixer.h"
#include "Asset/AssetManager.h"
#include "Asset/Audio/Music.h"
#include "Asset/Audio/SoundEffect.h"
#include <SDL2/SDL_mixer.h>

namespace toy {

SoundMixer::SoundMixer(AssetManager* assetManager)
: mAssetManager(assetManager)
, mBgmEnabled(true)
, mSoundEnabled(true)
, mVolume(0.3f)
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_VolumeMusic(static_cast<int>(mVolume * MIX_MAX_VOLUME));
}

SoundMixer::~SoundMixer()
{
    Mix_CloseAudio();
}

void SoundMixer::SetBGMEnable(bool enable)
{
    mBgmEnabled = enable;
}

void SoundMixer::SetSoundEnable(bool enable)
{
    mSoundEnabled = enable;
}

void SoundMixer::SetVolume(float volume)
{
    mVolume = volume;
    
    Mix_VolumeMusic(static_cast<int>(mVolume * MIX_MAX_VOLUME));
    // 効果音のボリュームは、個別再生時に適用する設計にしてる
}

bool SoundMixer::LoadBGM(const std::string& fileName)
{
    mCurrentBGM = mAssetManager->GetMusic(fileName);
    return mCurrentBGM != nullptr;
}

void SoundMixer::PlayBGM()
{
    if (mBgmEnabled && mCurrentBGM)
    {
        mCurrentBGM->Play(-1); // ループ再生
    }
}

void SoundMixer::StopBGM()
{
    Mix_HaltMusic();
}

void SoundMixer::PlaySoundEffect(const std::string& fileName)
{
    if (mSoundEnabled)
    {
        auto sound = mAssetManager->GetSoundEffect(fileName);
        if (sound)
        {
            Mix_VolumeChunk(sound->GetChunk(), static_cast<int>(mVolume * MIX_MAX_VOLUME));
            sound->Play();
        }
    }
}

} // namespace toy
*/
