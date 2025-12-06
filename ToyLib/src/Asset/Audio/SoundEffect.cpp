#include "Asset/Audio/SoundEffect.h"
#include "Asset/AssetManager.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>

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
        std::cerr << "[SoundEffect] Failed to load wav: " << fullPath.c_str() << std::endl;
        return false;
    }

    alGenBuffers(1, &mBuffer);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        std::cerr << "[SoundEffect] alGenBuffers error: " << err << std::endl;;
        mBuffer = 0;
        return false;
    }

    alBufferData(mBuffer,
                 format,
                 data.data(),
                 static_cast<ALsizei>(data.size()),
                 freq);

    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        std::cerr << "[SoundEffect] alBufferData error: " << err << "(file=" << fullPath.c_str() << std::endl;;
        alDeleteBuffers(1, &mBuffer);
        mBuffer = 0;
        return false;
    }

    return true;
}

//------------------------------------------
// RIFF チャンクを読む WAV ローダ
//------------------------------------------
struct RiffHeader
{
    char     id[4];      // "RIFF"
    uint32_t size;       // 全体サイズ - 8
    char     wave[4];    // "WAVE"
};

struct ChunkHeader
{
    char     id[4];      // "fmt " とか "data"
    uint32_t size;       // チャンクサイズ
};

bool SoundEffect::LoadWav16(const std::string& fullPath,
                            std::vector<char>& outData,
                            ALenum& outFormat,
                            ALsizei& outFreq)
{
    FILE* fp = std::fopen(fullPath.c_str(), "rb");
    if (!fp)
    {
        std::cerr << "[SoundEffect] fopen failed: " << fullPath.c_str() << std::endl;
        return false;
    }

    RiffHeader riff{};
    if (std::fread(&riff, sizeof(RiffHeader), 1, fp) != 1)
    {
        std::fclose(fp);
        return false;
    }

    if (std::strncmp(riff.id, "RIFF", 4) != 0 ||
        std::strncmp(riff.wave, "WAVE", 4) != 0)
    {
        std::cerr << "[SoundEffect] Not a RIFF/WAVE file: " << fullPath.c_str() << std::endl;
        std::fclose(fp);
        return false;
    }

    bool foundFmt  = false;
    bool foundData = false;

    uint16_t audioFormat   = 0;  // PCM = 1
    uint16_t numChannels   = 0;
    uint32_t sampleRate    = 0;
    uint16_t bitsPerSample = 0;

    std::vector<char> dataChunk;

    // チャンクを順番に走査
    while (!foundData)
    {
        ChunkHeader ch{};
        if (std::fread(&ch, sizeof(ChunkHeader), 1, fp) != 1)
        {
            break; // EOF or error
        }

        if (std::strncmp(ch.id, "fmt ", 4) == 0)
        {
            // fmt チャンク
            // 基本の PCM フォーマット
            struct FmtChunkBase
            {
                uint16_t audioFormat;
                uint16_t numChannels;
                uint32_t sampleRate;
                uint32_t byteRate;
                uint16_t blockAlign;
                uint16_t bitsPerSample;
            } fmt{};

            const size_t toRead = (ch.size < sizeof(FmtChunkBase))
                ? ch.size
                : sizeof(FmtChunkBase);

            if (std::fread(&fmt, toRead, 1, fp) != 1)
            {
                std::fclose(fp);
                return false;
            }

            // 残りがあればスキップ
            if (ch.size > toRead)
            {
                std::fseek(fp, ch.size - toRead, SEEK_CUR);
            }

            audioFormat   = fmt.audioFormat;
            numChannels   = fmt.numChannels;
            sampleRate    = fmt.sampleRate;
            bitsPerSample = fmt.bitsPerSample;

            foundFmt = true;
        }
        else if (std::strncmp(ch.id, "data", 4) == 0)
        {
            // data チャンク本体
            if (!foundFmt)
            {
                std::cerr << "[SoundEffect] data chunk before fmt chunk: " << fullPath.c_str() << std::endl;
                std::fclose(fp);
                return false;
            }

            dataChunk.resize(ch.size);
            if (ch.size > 0)
            {
                if (std::fread(dataChunk.data(), ch.size, 1, fp) != 1)
                {
                    std::fclose(fp);
                    return false;
                }
            }

            foundData = true;
        }
        else
        {
            // その他のチャンクはスキップ
            std::fseek(fp, ch.size, SEEK_CUR);
        }
    }

    std::fclose(fp);

    if (!foundFmt || !foundData)
    {
        std::cerr << "[SoundEffect] Missing fmt or data chunk: " << fullPath.c_str() << std::endl;
        return false;
    }

    if (audioFormat != 1)
    {
        std::cerr << "[SoundEffect] Non-PCM format not supported: " << fullPath.c_str() << std::endl;
        return false;
    }

    if (bitsPerSample != 8 && bitsPerSample != 16)
    {
        std::cerr << "[SoundEffect] Only 8/16bit supported: " << fullPath.c_str() << "(" << bitsPerSample << " bits)" << std::endl;
        return false;
    }

    if (numChannels < 1 || numChannels > 2)
    {
        std::cerr << "[SoundEffect] Only mono/stereo supported: " << fullPath.c_str() << " (ch=" << numChannels << ")" << std::endl;
        return false;
    }

    outFreq   = static_cast<ALsizei>(sampleRate);
    outData   = std::move(dataChunk);

    if (numChannels == 1 && bitsPerSample == 8)  outFormat = AL_FORMAT_MONO8;
    if (numChannels == 1 && bitsPerSample == 16) outFormat = AL_FORMAT_MONO16;
    if (numChannels == 2 && bitsPerSample == 8)  outFormat = AL_FORMAT_STEREO8;
    if (numChannels == 2 && bitsPerSample == 16) outFormat = AL_FORMAT_STEREO16;

    return true;
}

} // namespace toy
