// Audio/AudioSystemOpenAL.h
#pragma once
#include "Utils/MathUtil.h"
#include "Asset/Audio/SoundEffect.h"
#include "Asset/Audio/Music.h"

#include <memory>
#include <vector>


#include <AL/al.h>
#include <AL/alc.h>


namespace toy {

class AudioSystemOpenAL
{
public:
    AudioSystemOpenAL();
    ~AudioSystemOpenAL();

    bool Init();
    void Shutdown();

    void Update(float deltaTime);

    // BGM：Music アセットを受け取る
    bool PlayBGM(const std::shared_ptr<Music>& music, bool loop = true);
    void StopBGM();
    void SetBGMVolume(float volume);

    // SE：SoundEffect アセットを受け取る
    void PlaySE2D(const std::shared_ptr<SoundEffect>& se, float volume = 1.0f);
    void PlaySE3D(const std::shared_ptr<SoundEffect>& se,
                  const Vector3& worldPos,
                  float volume = 1.0f);

    void SetListenerTransform(const Vector3& pos,
                              const Vector3& forward,
                              const Vector3& up);

    void SetMasterVolume(float volume);

private:
    struct StreamingMP3BGM
    {
        static constexpr int NUM_BUFFERS = 4;
        static constexpr int CHUNK_SIZE  = 4096 * 4;

        std::shared_ptr<Music> music;        // AssetManager 管理の Music アセット
        ALuint source = 0;
        ALuint buffers[NUM_BUFFERS]{};
        std::vector<unsigned char> decodeBuf;
        bool loop        = true;
        bool initialized = false;

        bool Init(const std::shared_ptr<Music>& m);
        void Play();
        void Stop();
        void Update();
        void Shutdown();
        void SetVolume(float volume);
    };

    struct ActiveSource
    {
        ALuint source = 0;
    };

    ALCdevice*  mDevice  = nullptr;
    ALCcontext* mContext = nullptr;

    float mMasterVolume = 1.0f;

    StreamingMP3BGM             mBGM;
    std::vector<ActiveSource>   mActiveSources;
};

} // namespace toy
