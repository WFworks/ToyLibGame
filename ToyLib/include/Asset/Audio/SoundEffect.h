// Asset/Audio/SoundEffect.h
#pragma once
#include <string>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>

namespace toy {

class AssetManager;

class SoundEffect
{
public:
    SoundEffect();
    ~SoundEffect();

    bool Load(const std::string& fileName, AssetManager* manager);

    ALuint GetBuffer() const { return mBuffer; }

private:
    ALuint mBuffer;
    std::string mFilePath;

    bool LoadWav16(const std::string& fullPath,
                   std::vector<char>& outData,
                   ALenum& outFormat,
                   ALsizei& outFreq);
};

} // namespace toy


/*
#include <string>
#include <SDL2/SDL_mixer.h>

namespace toy {

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

} // namespace toy
*/
