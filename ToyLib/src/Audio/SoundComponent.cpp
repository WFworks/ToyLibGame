// Audio/SoundComponent.cpp
#include "Audio/SoundComponent.h"

#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Asset/AssetManager.h"
#include "Asset/Audio/SoundEffect.h"
#include "Engine/Render/Renderer.h"
#include "Utils/MathUtil.h"
#include <iostream>

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
    if (mIsExclusive && IsPlaying()) return;

    auto* app    = GetOwner()->GetApp();
    auto* assets = app->GetAssetManager();
    auto sound   = assets->GetSoundEffect(mSoundName);
    if (!sound) return;

    if (mSource == 0)
    {
        alGenSources(1, &mSource);

        // 一回だけでいい設定
        alSourcef(mSource, AL_REFERENCE_DISTANCE, 3.0f);  // ここまではほぼフル音量
        alSourcef(mSource, AL_MAX_DISTANCE, 50.0f);       // これ以上はあまり変わらない
        alSourcef(mSource, AL_ROLLOFF_FACTOR, 1.0f);      // 減衰の強さ
    }

    alSourcei(mSource, AL_BUFFER, sound->GetBuffer());
    alSourcef(mSource, AL_GAIN, mVolume);
    alSourcei(mSource, AL_LOOPING, mIsLoop ? AL_TRUE : AL_FALSE);

    // 位置だけ OpenAL に渡す
    auto pos = GetOwner()->GetPosition();
    alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);

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

    if (mSource != 0)
    {
        ALint state = 0;
        alGetSourcei(mSource, AL_SOURCE_STATE, &state);

        // 再生中なら Actor の位置に追従させる
        if (state == AL_PLAYING)
        {
            auto pos = GetOwner()->GetPosition();
            alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);
        }

        // ループしない場合、再生終了後にフラグをリセットしたりイベント飛ばしたりできる
        if (!mIsLoop && state == AL_STOPPED)
        {
            // TODO: 必要ならここで何かイベントを飛ばす or フラグをリセット
            // 例:
            // mHasPlayed = false;
        }
    }
}

} // namespace toy

