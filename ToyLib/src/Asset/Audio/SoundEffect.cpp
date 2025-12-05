// Asset/Audio/SoundEffect.cpp
#include "Asset/Audio/SoundEffect.h"
#include "Asset/AssetManager.h"
#include <cstdio>

namespace toy {

SoundEffect::SoundEffect()
: mBuffer(0)
{
}

SoundEffect::~SoundEffect()
{
    if (mBuffer != 0)
    {
        alDeleteBuffers(1, &mBuffer);
        mBuffer = 0;
    }
}

bool SoundEffect::Load(const std::string& fileName, AssetManager* manager)
{
    const std::string fullPath = manager->GetAssetsPath() + fileName;
    mFilePath = fullPath;

    std::vector<char> data;
    ALenum  format = 0;
    ALsizei freq   = 0;

    if (!LoadWav16(fullPath, data, format, freq))
    {
        return false;
    }

    alGenBuffers(1, &mBuffer);
    alBufferData(mBuffer,
                 format,
                 data.data(),
                 static_cast<ALsizei>(data.size()),
                 freq);

    return true;
}

bool SoundEffect::LoadWav16(const std::string& fullPath,
                            std::vector<char>& outData,
                            ALenum& outFormat,
                            ALsizei& outFreq)
{
    FILE* fp = std::fopen(fullPath.c_str(), "rb");
    if (!fp) return false;

    char riff[4];
    std::fread(riff, 1, 4, fp);

    std::fseek(fp, 22, SEEK_SET);
    short channels = 0;
    std::fread(&channels, 2, 1, fp);

    std::fread(&outFreq, 4, 1, fp);

    std::fseek(fp, 34, SEEK_SET);
    short bits = 0;
    std::fread(&bits, 2, 1, fp);

    char dataId[4];
    int  dataSize = 0;
    std::fseek(fp, 36, SEEK_SET);
    std::fread(dataId, 1, 4, fp);
    std::fread(&dataSize, 4, 1, fp);

    outData.resize(dataSize);
    std::fread(outData.data(), 1, dataSize, fp);

    std::fclose(fp);

    if (channels == 1 && bits == 8)  outFormat = AL_FORMAT_MONO8;
    if (channels == 1 && bits == 16) outFormat = AL_FORMAT_MONO16;
    if (channels == 2 && bits == 8)  outFormat = AL_FORMAT_STEREO8;
    if (channels == 2 && bits == 16) outFormat = AL_FORMAT_STEREO16;

    return true;
}

} // namespace toy
/*
#include "Asset/Audio/SoundEffect.h"
#include "Asset/AssetManager.h"

namespace toy {

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

} // namespace toy
*/
