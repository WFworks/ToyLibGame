// Asset/Audio/Music.cpp
#include "Asset/Audio/Music.h"
#include "Asset/AssetManager.h"

namespace toy {

int Music::sRefCount = 0;

Music::Music()
: mHandle(nullptr)
, mRate(0)
, mChannels(0)
, mEncoding(0)
{
}

Music::~Music()
{
    if (mHandle)
    {
        mpg123_close(mHandle);
        mpg123_delete(mHandle);
        mHandle = nullptr;

        ShutdownLib();
    }
}

void Music::InitLib()
{
    if (sRefCount == 0)
    {
        mpg123_init();
    }
    ++sRefCount;
}

void Music::ShutdownLib()
{
    --sRefCount;
    if (sRefCount <= 0)
    {
        sRefCount = 0;
        mpg123_exit();
    }
}

bool Music::Load(const std::string& fileName, AssetManager* manager)
{
    // アセットパスを解決してフルパスとして保持
    mFilePath = manager->GetAssetsPath() + fileName;

    // mpg123 ライブラリ初期化（参照カウント制御）
    InitLib();

    int err = 0;
    mHandle = mpg123_new(nullptr, &err);
    if (!mHandle)
    {
        ShutdownLib();
        return false;
    }

    if (mpg123_open(mHandle, mFilePath.c_str()) != MPG123_OK)
    {
        mpg123_delete(mHandle);
        mHandle = nullptr;
        ShutdownLib();
        return false;
    }

    // 出力フォーマットを OpenAL 向けに固定（16bit stereo）
    mpg123_format_none(mHandle);
    mpg123_format(mHandle,
                  44100,
                  MPG123_STEREO,
                  MPG123_ENC_SIGNED_16);

    mpg123_getformat(mHandle, &mRate, &mChannels, &mEncoding);

    return true;
}

void Music::Rewind()
{
    if (!mHandle) return;
    mpg123_seek(mHandle, 0, SEEK_SET);
}

size_t Music::ReadChunk(unsigned char* out, size_t chunkSize)
{
    if (!mHandle) return 0;

    size_t done = 0;
    int res = mpg123_read(mHandle, out, chunkSize, &done);

    if (res == MPG123_DONE) return 0;
    if (res != MPG123_OK)   return 0;

    return done;
}

} // namespace toy
