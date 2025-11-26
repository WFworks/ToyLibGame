#include "SoundEffect.h"
#include "AssetManager.h"

SoundEffect::SoundEffect()
    : mChunk(nullptr)
{
}

SoundEffect::~SoundEffect()
{
    if (mChunk)
    {
        Mix_FreeChunk(mChunk);
    }
}

bool SoundEffect::Load(const std::string& fileName, AssetManager* assetManager)
{
    std::string fullName = assetManager->GetAssetsPath() + fileName;
    mChunk = Mix_LoadWAV(fullName.c_str());
    return mChunk != nullptr;
}

void SoundEffect::Play(int loops)
{
    if (mChunk)
    {
        Mix_PlayChannel(-1, mChunk, loops);
    }
}

