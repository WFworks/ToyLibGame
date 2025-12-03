#pragma once
#include <string>
#include <SDL2/SDL_mixer.h>

class SoundEffect
{
public:
    SoundEffect();
    ~SoundEffect();

    bool Load(const std::string& fileName, class AssetManager* assetManager);
    void Play(int loops = 0);

    Mix_Chunk* GetChunk() const { return mChunk; }

private:
    Mix_Chunk* mChunk;
};
