#pragma once

namespace toy::kit {

class Game : public toy::Application
{
public:
    Game();
    virtual ~Game() = default;

protected:
    // ゲーム開始時に1回だけ呼ばれる
    virtual void Setup() {}

    // シーンや共通処理を更新
    void InitGame() override;
    void UpdateGame(float dt) override;
    void ShutdownGame() override;

    // Kitの中核サービス
    class SceneManager& scenes();
    class InputMap&     input();
};

} // namespace toy::kit
