#include "Asset/Audio/Music.h"
#include "Asset/AssetManager.h"

Music::Music()
    : mMusic(nullptr)
{
}

Music::~Music()
{
    if (mMusic)
    {
        Mix_FreeMusic(mMusic);
    }
}

bool Music::Load(const std::string& fileName, AssetManager* assetManager)
{
    std::string fullName = assetManager->GetAssetsPath() + fileName;
    mMusic = Mix_LoadMUS(fullName.c_str());
    return mMusic != nullptr;
}

void Music::Play(int loops)
{
    if (mMusic)
    {
        Mix_PlayMusic(mMusic, loops);
    }
}

void Music::Stop()
{
    Mix_HaltMusic();
}
