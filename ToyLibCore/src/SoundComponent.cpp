#include "SoundComponent.h"
#include "Actor.h"
#include "Application.h"
#include "AssetManager.h"
#include "SoundEffect.h"
#include "Renderer.h"

SoundComponent::SoundComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mVolume(1.0f)
    , mIsLoop(false)
    , mAutoPlay(false)
    , mUseDistanceAttenuation(false)
    , mIsExclusive(false)
    , mChannel(-1)
    , mHasPlayed(false)
{
}

SoundComponent::~SoundComponent()
{
    Stop(); // 再生中の音を止める
}

void SoundComponent::SetSound(const std::string& fileName)
{
    mSoundName = fileName;
}

void SoundComponent::Play()
{
    if (mSoundName.empty()) return;

    if (mIsExclusive && IsPlaying())
    {
        return; // 排他再生中は新しく鳴らさない
    }

    auto sound = mOwnerActor->GetApp()->GetAssetManager()->GetSoundEffect(mSoundName);
    if (!sound) return;

    float actualVolume = mVolume;

    if (mUseDistanceAttenuation)
    {
        Vector3 camPos = mOwnerActor->GetApp()->GetRenderer()->GetViewMatrix().GetTranslation();
        Vector3 pos = mOwnerActor->GetPosition();
        float distance = (camPos - pos).Length();

        if (distance > 5.0f)
        {
            actualVolume *= (5.0f / distance);
            if (actualVolume < 0.0f) actualVolume = 0.0f;
        }
    }

    Mix_VolumeChunk(sound->GetChunk(), static_cast<int>(actualVolume * MIX_MAX_VOLUME));
    mChannel = Mix_PlayChannel(-1, sound->GetChunk(), mIsLoop ? -1 : 0);
}

void SoundComponent::Stop()
{
    if (mChannel >= 0)
    {
        Mix_HaltChannel(mChannel);
        mChannel = -1;
    }
}

bool SoundComponent::IsPlaying() const
{
    return mChannel >= 0 && Mix_Playing(mChannel) == 1;
}

void SoundComponent::Update(float deltaTime)
{
    if (mAutoPlay && !mHasPlayed)
    {
        Play();
        mHasPlayed = true;
    }

    // チャンネルが停止してたら -1 にリセット（明示的に管理）
    if (mChannel >= 0 && !IsPlaying())
    {
        mChannel = -1;
    }
}
