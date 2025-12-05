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
    ALuint mBuffer = 0;
    std::string mFilePath;

    bool LoadWav16(const std::string& fullPath,
                   std::vector<char>& outData,
                   ALenum& outFormat,
                   ALsizei& outFreq);
};


} // namespace toy
