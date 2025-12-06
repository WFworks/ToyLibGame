#pragma once

#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <string>

namespace toy {

//---------------------------------------------
// アプリケーションの基盤クラス
// ・ウィンドウ／描画／入力／物理などの主要システムを管理
// ・ゲームごとに派生し、InitGame/UpdateGame でロジックを記述
//---------------------------------------------
class Application
{
public:
    Application();
    virtual ~Application();
    
    // アプリ全体の初期化（SDL, Renderer, 各Subsystem 初期化）
    virtual bool Initialize();
    
    // メインループ（ProcessInput → UpdateFrame → Draw）
    void RunLoop();
    
    // 全解放
    void Shutdown();
    
    //-----------------------------------------
    // Actor 管理
    //-----------------------------------------
    
    // 既存 Actor を登録
    void AddActor(std::unique_ptr<class Actor> a);
    
    // Actor を生成して登録（CreateActor<T>()）
    template <typename T, typename... Args>
    T* CreateActor(Args&&... args)
    {
        auto actor = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* rawPtr = actor.get();
        AddActor(std::move(actor));
        return rawPtr;
    }

    // Actor を削除予約（即時削除ではなく安全なタイミングで破棄）
    void DestroyActor(class Actor* actor);
    
    //-----------------------------------------
    // システム取得
    //-----------------------------------------
    class Renderer*        GetRenderer()        const { return mRenderer.get(); }
    class PhysWorld*       GetPhysWorld()       const { return mPhysWorld.get(); }
    class AssetManager*    GetAssetManager()    const { return mAssetManager.get(); }
    class SoundMixer*      GetSoundMixer()      const { return mSoundMixer.get(); }
    class TimeOfDaySystem* GetTimeOfDaySystem() const { return mTimeOfDaySys.get(); }
    
protected:
    //-----------------------------------------
    // ゲーム側でオーバーライドするフック関数
    //-----------------------------------------
    
    // 毎フレームの更新
    virtual void UpdateGame(float deltaTime) { }
    
    // 初期ロード（Actor 生成など）
    virtual void InitGame() {}
    
    // シャットダウン処理
    virtual void ShutdownGame() {}

    // アセットマネージャの初期化（パス設定と DPI）
    void InitAssetManager(const std::string& path, float dpi = 1.0f);
    
private:
    //-----------------------------------------
    // 内部処理（ゲームループ関連）
    //-----------------------------------------
    
    // データロード & 解放
    void LoadData();
    void UnloadData();
    
    // 入力処理
    void ProcessInput();
    
    // 1フレーム更新（アクター処理など）
    void UpdateFrame();
    
    // 描画
    void Draw();
    
    //-----------------------------------------
    // ウィンドウ／アプリ設定
    //-----------------------------------------
    
    std::string mApplicationTitle; // ウィンドウタイトル
    bool  mIsFullScreen;           // フルスクリーン状態
    float mScreenW;                // ウィンドウ横幅
    float mScreenH;                // ウィンドウ縦幅
    bool  mIsActive;               // 実行中フラグ
    bool  mIsPause;                // 一時停止フラグ
    Uint64 mTicksCount;            // フレーム時間計測
    
    //-----------------------------------------
    // サブシステム
    //-----------------------------------------
    
    std::unique_ptr<class Renderer>        mRenderer;
    std::unique_ptr<class InputSystem>     mInputSys;
    std::unique_ptr<class PhysWorld>       mPhysWorld;
    std::unique_ptr<class AssetManager>    mAssetManager;
    std::unique_ptr<class SoundMixer>      mSoundMixer;
    std::unique_ptr<class TimeOfDaySystem> mTimeOfDaySys;
    
    //-----------------------------------------
    // Actor 管理
    //-----------------------------------------
    
    std::vector<std::unique_ptr<class Actor>> mActors;         // アクティブな Actor
    std::vector<std::unique_ptr<class Actor>> mPendingActors;  // 追加待ち（二重更新防止）
    bool mIsUpdatingActors;                                     // 更新中フラグ
};

} // namespace toy
