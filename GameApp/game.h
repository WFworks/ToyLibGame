#include "ToyLib.h"
#include <string>
#include <memory>

class Game : public Application
{
public:
    Game();
    ~Game();
protected:
    void InitGame() override;
    void LoadData();
    void UpdateGame(float deltaTime) override;
    void ShutdownGame() override;
private:
    std::unique_ptr<class WeatherManager> mWeather;

};
