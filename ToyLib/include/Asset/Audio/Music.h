// Asset/Audio/Music.h
#pragma once
#include <string>
#include <cstddef>
#include <memory>

#include <mpg123.h>

namespace toy {

class AssetManager;

// 「BGM用 MP3 アセット」
// - AssetManager によって管理される
// - デコード状態（mpg123_handle）もこの中で保持
class Music
{
public:
    Music();
    ~Music();

    // AssetManager から呼ばれる
    bool Load(const std::string& fileName, AssetManager* manager);

    // ストリーミング用 API（AudioSystem から利用）
    void   Rewind();
    size_t ReadChunk(unsigned char* out, size_t chunkSize);

    long GetRate() const    { return mRate; }
    int  GetChannels() const{ return mChannels; }

    const std::string& GetFilePath() const { return mFilePath; }

private:
    std::string    mFilePath;
    mpg123_handle* mHandle   = nullptr;
    long           mRate     = 0;
    int            mChannels = 0;
    int            mEncoding = 0;

    // mpg123 のグローバル初期化を管理するための静的カウンタ
    static int  sRefCount;
    static void InitLib();
    static void ShutdownLib();
};

} // namespace toy} // namespace toy


/*
#pragma once
#include <string>
#include <SDL2/SDL_mixer.h>

namespace toy {

class Music
{
public:
    Music();
    ~Music();
    
    bool Load(const std::string& fileName, class AssetManager* assetManager);
    void Play(int loops = -1);
    void Stop();
    
private:
    Mix_Music* mMusic;
};

} //  namespace toy

*/
