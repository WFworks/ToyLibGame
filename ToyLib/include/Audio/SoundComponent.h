// Audio/SoundComponent.h
#pragma once
#include "Engine/Core/Component.h"
#include <string>

#if defined(__APPLE__)
    #include <AL/al.h>
    #include <AL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

namespace toy {

class SoundComponent : public Component
{
public:
    SoundComponent(class Actor* owner, int updateOrder = 100);
    ~SoundComponent();

    void SetSound(const std::string& fileName);
    void Play();
    void Stop();
    bool IsPlaying() const;

    void SetVolume(float volume) { mVolume = volume; }
    void SetLoop(bool loop) { mIsLoop = loop; }
    void SetAutoPlay(bool autoPlay) { mAutoPlay = autoPlay; }
    void SetUseDistanceAttenuation(bool useAttenuation) { mUseDistanceAttenuation = useAttenuation; }
    void SetExclusive(bool isExclusive) { mIsExclusive = isExclusive; }

    void Update(float deltaTime) override;

private:
    std::string mSoundName;
    float mVolume;
    bool  mIsLoop;
    bool  mAutoPlay;
    bool  mUseDistanceAttenuation;
    bool  mIsExclusive;

    bool  mHasPlayed;

    ALuint mSource; // このコンポーネント専用のソース
};

} // namespace toy

