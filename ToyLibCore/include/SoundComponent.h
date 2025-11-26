#pragma once
#include "Component.h"
#include <string>

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
    bool mIsLoop;
    bool mAutoPlay;
    bool mUseDistanceAttenuation;
    bool mIsExclusive;

    int mChannel;
    bool mHasPlayed;
};
