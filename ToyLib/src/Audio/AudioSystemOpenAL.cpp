// Audio/AudioSystemOpenAL.cpp
#include "Audio/AudioSystemOpenAL.h"
#include <algorithm>

namespace toy {

//==============================
// StreamingMP3BGM
//==============================

bool AudioSystemOpenAL::StreamingMP3BGM::Init(const std::shared_ptr<Music>& m)
{
    music = m;
    if (!music)
    {
        return false;
    }

    music->Rewind();

    alGenSources(1, &source);
    alGenBuffers(NUM_BUFFERS, buffers);

    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    decodeBuf.resize(CHUNK_SIZE);

    for (int i = 0; i < NUM_BUFFERS; ++i)
    {
        size_t bytes = music->ReadChunk(decodeBuf.data(), CHUNK_SIZE);
        if (bytes == 0) break;

        ALenum format = (music->GetChannels() == 2)
            ? AL_FORMAT_STEREO16
            : AL_FORMAT_MONO16;

        alBufferData(buffers[i],
                     format,
                     decodeBuf.data(),
                     static_cast<ALsizei>(bytes),
                     static_cast<ALsizei>(music->GetRate()));

        alSourceQueueBuffers(source, 1, &buffers[i]);
    }

    initialized = true;
    return true;
}

void AudioSystemOpenAL::StreamingMP3BGM::Play()
{
    if (!initialized) return;
    alSourcePlay(source);
}

void AudioSystemOpenAL::StreamingMP3BGM::Stop()
{
    if (!initialized) return;
    alSourceStop(source);
}

void AudioSystemOpenAL::StreamingMP3BGM::Update()
{
    if (!initialized || !music) return;

    ALint processed = 0;
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

    while (processed-- > 0)
    {
        ALuint buf = 0;
        alSourceUnqueueBuffers(source, 1, &buf);

        size_t bytes = music->ReadChunk(decodeBuf.data(), CHUNK_SIZE);

        if (bytes > 0)
        {
            ALenum format = (music->GetChannels() == 2)
                ? AL_FORMAT_STEREO16
                : AL_FORMAT_MONO16;

            alBufferData(buf,
                         format,
                         decodeBuf.data(),
                         static_cast<ALsizei>(bytes),
                         static_cast<ALsizei>(music->GetRate()));

            alSourceQueueBuffers(source, 1, &buf);
        }
        else
        {
            // 終端。ループなら巻き戻して再デコード
            if (loop)
            {
                music->Rewind();
                bytes = music->ReadChunk(decodeBuf.data(), CHUNK_SIZE);

                if (bytes > 0)
                {
                    ALenum format = (music->GetChannels() == 2)
                        ? AL_FORMAT_STEREO16
                        : AL_FORMAT_MONO16;

                    alBufferData(buf,
                                 format,
                                 decodeBuf.data(),
                                 static_cast<ALsizei>(bytes),
                                 static_cast<ALsizei>(music->GetRate()));

                    alSourceQueueBuffers(source, 1, &buf);
                }
            }
        }
    }
}

void AudioSystemOpenAL::StreamingMP3BGM::Shutdown()
{
    if (!initialized) return;

    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(NUM_BUFFERS, buffers);

    music.reset();
    initialized = false;
}

void AudioSystemOpenAL::StreamingMP3BGM::SetVolume(float volume)
{
    if (!initialized) return;
    alSourcef(source, AL_GAIN, volume);
}

//==============================
// AudioSystemOpenAL
//==============================

AudioSystemOpenAL::AudioSystemOpenAL()
{
}

AudioSystemOpenAL::~AudioSystemOpenAL()
{
    Shutdown();
}

bool AudioSystemOpenAL::Init()
{
    mDevice = alcOpenDevice(nullptr);
    if (!mDevice)
    {
        return false;
    }

    mContext = alcCreateContext(mDevice, nullptr);
    if (!mContext)
    {
        alcCloseDevice(mDevice);
        mDevice = nullptr;
        return false;
    }

    alcMakeContextCurrent(mContext);
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    mMasterVolume = 1.0f;
    return true;
}

void AudioSystemOpenAL::Shutdown()
{
    mBGM.Shutdown();

    for (auto& s : mActiveSources)
    {
        if (s.source != 0)
        {
            alSourceStop(s.source);
            alDeleteSources(1, &s.source);
        }
    }
    mActiveSources.clear();

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

void AudioSystemOpenAL::Update(float)
{
    mBGM.Update();

    std::vector<ActiveSource> stillActive;
    stillActive.reserve(mActiveSources.size());

    for (auto& s : mActiveSources)
    {
        ALint state = 0;
        alGetSourcei(s.source, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED)
        {
            alDeleteSources(1, &s.source);
        }
        else
        {
            stillActive.push_back(s);
        }
    }

    mActiveSources.swap(stillActive);
}

bool AudioSystemOpenAL::PlayBGM(const std::shared_ptr<Music>& music, bool loop)
{
    if (!music) return false;

    mBGM.Shutdown();

    mBGM.loop = loop;
    if (!mBGM.Init(music))
    {
        return false;
    }

    mBGM.SetVolume(mMasterVolume);
    mBGM.Play();
    return true;
}

void AudioSystemOpenAL::StopBGM()
{
    mBGM.Stop();
}

void AudioSystemOpenAL::SetBGMVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    mBGM.SetVolume(volume * mMasterVolume);
}

void AudioSystemOpenAL::PlaySE2D(const std::shared_ptr<SoundEffect>& se, float volume)
{
    if (!se) return;

    ALuint src = 0;
    alGenSources(1, &src);

    alSourcei(src, AL_BUFFER, se->GetBuffer());
    alSource3f(src, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSourcef(src, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcef(src, AL_GAIN, volume * mMasterVolume);

    alSourcePlay(src);

    ActiveSource s;
    s.source = src;
    mActiveSources.push_back(s);
}

void AudioSystemOpenAL::PlaySE3D(const std::shared_ptr<SoundEffect>& se,
                                 const Vector3& worldPos,
                                 float volume)
{
    if (!se) return;

    ALuint src = 0;
    alGenSources(1, &src);

    alSourcei(src, AL_BUFFER, se->GetBuffer());
    alSource3f(src, AL_POSITION,
               worldPos.x, worldPos.y, worldPos.z);
    alSourcef(src, AL_ROLLOFF_FACTOR, 1.0f);
    alSourcef(src, AL_GAIN, volume * mMasterVolume);

    alSourcePlay(src);

    ActiveSource s;
    s.source = src;
    mActiveSources.push_back(s);
}

void AudioSystemOpenAL::SetListenerTransform(const Vector3& pos,
                                             const Vector3& forward,
                                             const Vector3& up)
{
    ALfloat listenerPos[] = { pos.x, pos.y, pos.z };
    ALfloat listenerOri[] = {
        forward.x, forward.y, forward.z,
        up.x,      up.y,      up.z
    };

    alListenerfv(AL_POSITION,    listenerPos);
    alListenerfv(AL_ORIENTATION, listenerOri);
}

void AudioSystemOpenAL::SetMasterVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    mMasterVolume = volume;
    mBGM.SetVolume(mMasterVolume);
}

} // namespace toy
