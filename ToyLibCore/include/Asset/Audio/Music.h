#pragma once
#include <string>
#include <SDL2/SDL_mixer.h>

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
