#include "SoundMixer.h"
#include "AssetManager.h"
#include "Music.h"
#include "SoundEffect.h"
#include <SDL2/SDL_mixer.h>

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
