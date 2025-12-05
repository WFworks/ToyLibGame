// Audio/SoundComponent.cpp
#include "Audio/SoundComponent.h"

#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Asset/AssetManager.h"
#include "Asset/Audio/SoundEffect.h"
#include "Engine/Render/Renderer.h"
#include "Utils/MathUtil.h"

namespace toy {

SoundComponent::SoundComponent(Actor* owner, int updateOrder)
: Component(owner, updateOrder)
, mVolume(1.0f)
, mIsLoop(false)
, mAutoPlay(false)
, mUseDistanceAttenuation(false)
, mIsExclusive(false)
, mHasPlayed(false)
, mSource(0)
{
}

SoundComponent::~SoundComponent()
{
    Stop();
    if (mSource != 0)
    {
        alDeleteSources(1, &mSource);
        mSource = 0;
    }
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
        return;
    }

    auto* app = GetOwner()->GetApp();
    auto* assets = app->GetAssetManager();
    auto sound = assets->GetSoundEffect(mSoundName);
    if (!sound) return;

    if (mSource == 0)
    {
        alGenSources(1, &mSource);
    }

    float actualVolume = mVolume;

    if (mUseDistanceAttenuation)
    {
        // カメラ位置を取得して簡易的な距離減衰
        auto camPos = app->GetRenderer()->GetViewMatrix().GetTranslation();
        auto pos    = GetOwner()->GetPosition();
        float distance = (camPos - pos).Length();

        const float refDist = 5.0f;
        if (distance > refDist)
        {
            actualVolume *= (refDist / distance);
            if (actualVolume < 0.0f) actualVolume = 0.0f;
        }
    }

    if (actualVolume < 0.0f) actualVolume = 0.0f;
    if (actualVolume > 1.0f) actualVolume = 1.0f;

    alSourcei(mSource, AL_BUFFER, sound->GetBuffer());
    alSourcef(mSource, AL_GAIN, actualVolume);
    alSourcei(mSource, AL_LOOPING, mIsLoop ? AL_TRUE : AL_FALSE);

    // 3D位置を Actor に合わせる（簡易）
    auto pos = GetOwner()->GetPosition();
    alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);
    alSourcef(mSource, AL_ROLLOFF_FACTOR,
              mUseDistanceAttenuation ? 1.0f : 0.0f);

    alSourcePlay(mSource);
}

void SoundComponent::Stop()
{
    if (mSource != 0)
    {
        alSourceStop(mSource);
    }
}

bool SoundComponent::IsPlaying() const
{
    if (mSource == 0) return false;

    ALint state = 0;
    alGetSourcei(mSource, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

void SoundComponent::Update(float)
{
    if (mAutoPlay && !mHasPlayed)
    {
        Play();
        mHasPlayed = true;
    }

    // ループしない場合、再生終了後にフラグをリセット
    if (!mIsLoop && mSource != 0)
    {
        ALint state = 0;
        alGetSourcei(mSource, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED)
        {
            // 必要ならここで何かイベントを飛ばしてもよい
        }
    }
}

} // namespace toy
/*
 #include "Audio/SoundComponent.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Asset/AssetManager.h"
#include "Asset/Audio/SoundEffect.h"
#include "Engine/Render/Renderer.h"

namespace toy {

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
    
    auto sound = GetOwner()->GetApp()->GetAssetManager()->GetSoundEffect(mSoundName);
    if (!sound) return;
    
    float actualVolume = mVolume;
    
    if (mUseDistanceAttenuation)
    {
        Vector3 camPos = GetOwner()->GetApp()->GetRenderer()->GetViewMatrix().GetTranslation();
        Vector3 pos = GetOwner()->GetPosition();
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

} // namespace toy
*/
