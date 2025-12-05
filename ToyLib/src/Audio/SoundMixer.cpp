// Audio/SoundMixer.cpp
#include "Audio/SoundMixer.h"

#include "Asset/AssetManager.h"
#include "Asset/Audio/Music.h"
#include "Asset/Audio/SoundEffect.h"

#include <cstdio>
#include <algorithm>

namespace toy {

SoundMixer::SoundMixer(AssetManager* assetManager)
: mAssetManager(assetManager)
, mDevice(nullptr)
, mContext(nullptr)
, mBgmSource(0)
, mBgmPlaying(false)
, mBgmLoop(true)
, mBgmEnabled(true)
, mSoundEnabled(true)
, mVolume(0.3f)
{
    InitOpenAL();
    mBgmDecodeBuffer.resize(BGM_CHUNK_SIZE);
}

SoundMixer::~SoundMixer()
{
    ShutdownBGMSource();

    // SE の後片付け
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
        printf("[SoundMixer] Failed to open device\n");
        return;
    }

    mContext = alcCreateContext(mDevice, nullptr);
    if (!mContext)
    {
        printf("[SoundMixer] Failed to create context\n");
        alcCloseDevice(mDevice);
        mDevice = nullptr;
        return;
    }

    alcMakeContextCurrent(mContext);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        printf("[SoundMixer] OpenAL error after init: 0x%x\n", err);
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

        alSource3f(mBgmSource, AL_POSITION, 0, 0, 0);
        alSourcef (mBgmSource, AL_ROLLOFF_FACTOR, 0.0f);
        alSourcei (mBgmSource, AL_LOOPING, AL_FALSE);
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
}

void SoundMixer::SetBGMEnable(bool enable)
{
    mBgmEnabled = enable;
    if (!mBgmEnabled)
        StopBGM();
}

void SoundMixer::SetSoundEnable(bool enable)
{
    mSoundEnabled = enable;
}

void SoundMixer::SetVolume(float volume)
{
    mVolume = std::clamp(volume, 0.0f, 1.0f);

    if (mBgmSource != 0)
        alSourcef(mBgmSource, AL_GAIN, mVolume);
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

    // 既存キュー消去
    alSourceStop(mBgmSource);
    ALint queued = 0;
    alGetSourcei(mBgmSource, AL_BUFFERS_QUEUED, &queued);
    while (queued-- > 0)
    {
        ALuint buf = 0;
        alSourceUnqueueBuffers(mBgmSource, 1, &buf);
    }

    mCurrentBGM->Rewind();

    // 最初の複数バッファを埋める
    for (int i = 0; i < BGM_NUM_BUFFERS; ++i)
    {
        size_t bytes = mCurrentBGM->ReadChunk(
            mBgmDecodeBuffer.data(),
            BGM_CHUNK_SIZE
        );
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
}

void SoundMixer::StopBGM()
{
    if (mBgmSource != 0)
    {
        alSourceStop(mBgmSource);
        mBgmPlaying = false;

        if (mCurrentBGM)
            mCurrentBGM->Rewind();
    }
}

void SoundMixer::PlaySoundEffect(const std::string& fileName)
{
    if (!mSoundEnabled) return;

    auto se = mAssetManager->GetSoundEffect(fileName);
    if (!se) return;

    ALuint src = 0;
    alGenSources(1, &src);

    alSourcei(src, AL_BUFFER, se->GetBuffer());
    alSourcef(src, AL_GAIN, mVolume);
    alSource3f(src, AL_POSITION, 0, 0, 0);
    alSourcef(src, AL_ROLLOFF_FACTOR, 0.0f);

    alSourcePlay(src);
    mOneShotSources.push_back(src);
}

void SoundMixer::Update(float,
                        const Matrix4& invViewMatrix)
{
    //====================
    // リスナー更新
    //====================
    Vector3 pos      = invViewMatrix.GetTranslation();
    Vector3 up       = invViewMatrix.GetYAxis();
    Vector3 forward  = invViewMatrix.GetZAxis();   // -Z が前向き
    forward = forward * -1.0f;


    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);

    float ori[6] = {
        forward.x, forward.y, forward.z,
        up.x,      up.y,      up.z
    };
    alListenerfv(AL_ORIENTATION, ori);

    float vel[3] = {0,0,0};
    alListenerfv(AL_VELOCITY, vel);

    //====================
    // BGM ストリーミング
    //====================
    if (mBgmPlaying && mCurrentBGM && mBgmSource != 0)
    {
        ALint processed = 0;
        alGetSourcei(mBgmSource, AL_BUFFERS_PROCESSED, &processed);

        while (processed-- > 0)
        {
            ALuint buf = 0;
            alSourceUnqueueBuffers(mBgmSource, 1, &buf);

            size_t bytes = mCurrentBGM->ReadChunk(
                mBgmDecodeBuffer.data(),
                BGM_CHUNK_SIZE
            );

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
                // ループ
                mCurrentBGM->Rewind();
                bytes = mCurrentBGM->ReadChunk(
                    mBgmDecodeBuffer.data(),
                    BGM_CHUNK_SIZE
                );

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

    //====================
    // SE 削除チェック
    //====================
    std::vector<ALuint> alive;
    alive.reserve(mOneShotSources.size());

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
            alive.push_back(src);
        }
    }

    mOneShotSources.swap(alive);
}

} // namespace toy
