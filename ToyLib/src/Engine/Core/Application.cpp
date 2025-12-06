#include "Engine/Core/Application.h"
#include "Engine/Core/Actor.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Runtime/InputSystem.h"
#include "Physics/PhysWorld.h"
#include "Asset/AssetManager.h"
#include "Audio/SoundMixer.h"
#include "Engine/Runtime/TimeOfDaySystem.h"

#include <algorithm>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>

namespace toy {

//=============================================================
// コンストラクタ／デストラクタ
//=============================================================

Application::Application()
: mIsActive(false)
, mIsUpdatingActors(false)
, mIsPause(false)
{
    // 各サブシステムを生成（所有は Application）
    mRenderer      = std::make_unique<Renderer>();
    mInputSys      = std::make_unique<InputSystem>();
    mPhysWorld     = std::make_unique<PhysWorld>();
    mAssetManager  = std::make_unique<AssetManager>();
    mSoundMixer    = std::make_unique<SoundMixer>(mAssetManager.get());
    mTimeOfDaySys  = std::make_unique<TimeOfDaySystem>();
}

// デストラクタ
Application::~Application()
{
    // 明示的な処理はなし（Shutdown() 内で片付ける前提）
}


//=============================================================
// 初期化／メインループ／終了処理
//=============================================================

// アプリ初期化
bool Application::Initialize()
{
    // SDL初期化（動画＋ゲームパッド）
    if ( !SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) )
    {
        std::cerr << "Failed to init SDL:" << SDL_GetError() << "" << std::endl;
        return false;
    }

    // SDL_ttf 初期化
    if (!TTF_Init())
    {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Renderer初期化（ウィンドウ・GLコンテキスト生成など）
    mRenderer->Initialize();
    
    // 入力システム初期化（Gamepadのオープン等）
    mInputSys->Initialize(mRenderer->GetSDLWindow());
    mInputSys->LoadButtonConfig("ToyLib/Settings/InputConfig.json");
    
    // データロード（主に Renderer / AssetManager 経由のリソース登録）
    LoadData();
    
    // ゲーム側（派生クラス）の初期化
    InitGame();
    
    mIsActive   = true;
    mTicksCount = SDL_GetTicks();
    
    return true;
}

// メインループ
void Application::RunLoop()
{
    while (mIsActive)
    {
        ProcessInput();
        UpdateFrame();
        Draw();
    }
}

// 描画処理（Renderer に委譲）
void Application::Draw()
{
    mRenderer->Draw();
}

// 終了処理
void Application::Shutdown()
{
    // ゲーム側の終了処理
    ShutdownGame();
    
    // リソース解放
    UnloadData();
    
    // サブシステム終了
    mInputSys->Shutdown();
    mRenderer->Shutdown();
    
    // SDL 系終了
    TTF_Quit();
    SDL_Quit();
}


//=============================================================
// 入力処理
//=============================================================

// 入力受付
void Application::ProcessInput()
{
    // 前フレーム状態を保存
    mInputSys->PrepareForUpdate();
    
    // SDL イベント処理
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            mIsActive = false;
            break;
        }
    }
    
    // 入力状態更新（キーボード／パッド）
    mInputSys->Update();
    const InputState& state = mInputSys->GetState();
    
    // ESCキーでアプリ終了
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_ESCAPE) == EReleased)
    {
        mIsActive = false;
    }
    
    // SPACE押しっぱなしでポーズ
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_SPACE) == EHeld)
    {
        mIsPause = true;
    }
    else
    {
        mIsPause = false;
    }
    
    // 全 Actor に入力を伝える
    for (auto& actor : mActors)
    {
        actor->ProcessInput(state);
    }
}


//=============================================================
// Actor 管理
//=============================================================

// Actor追加
void Application::AddActor(std::unique_ptr<Actor> actor)
{
    if (mIsUpdatingActors)
    {
        // 更新中は Pending に積んで、Update 後にまとめて反映
        mPendingActors.emplace_back(std::move(actor));
    }
    else
    {
        mActors.emplace_back(std::move(actor));
    }
}

// Actor を削除予約（実際の削除は UpdateFrame 内で）
void Application::DestroyActor(Actor* actor)
{
    if (actor)
    {
        actor->SetState(Actor::EDead);   // 実削除は Update 内で行う
    }
}


//=============================================================
// データロード／解放
//=============================================================

// データ解放
void Application::UnloadData()
{
    // すべての Actor を削除
    mActors.clear();
    
    // Renderer 内部リソース解放
    if (mRenderer)
    {
        mRenderer->UnloadData();
    }
    
    // AssetManager 内部リソース解放
    if (mAssetManager)
    {
        mAssetManager->UnloadData();
    }
}

// Actors, Renderer関連のロード（デフォルトは何もしない）
void Application::LoadData()
{
    // ゲーム側で必要に応じて override して使う前提
}


//=============================================================
// ゲームメインルーチン（1フレーム更新）
//=============================================================

// フレーム更新
void Application::UpdateFrame()
{
    //=====================================
    // 固定フレームレート (60fps 相当)
    //=====================================
    // 60fps ≒ 16ms
    const Uint64 frameDurationNS = 16'000'000;  // 16ms → ナノ秒
    Uint64 now = SDL_GetTicksNS();

    while ((now - mTicksCount) < frameDurationNS)
    {
        SDL_Delay(1); // CPU負荷を軽減
        now = SDL_GetTicksNS();
    }

    // 経過時間を秒に変換
    float deltaTime = (now - mTicksCount) / 1'000'000'000.0f; // ns → 秒
    if (deltaTime > 0.05f)
    {
        // フレーム飛びなどを考慮して最大値を制限
        deltaTime = 0.05f;
    }

    mTicksCount = now;
    
    // ポーズ中はここで更新をスキップ
    if (mIsPause)
        return;
    
    //=====================================
    // 時間経過（昼夜サイクル等）
    //=====================================
    mTimeOfDaySys->Update(deltaTime);
    
    //=====================================
    // ゲームロジック更新（派生クラス）
    //=====================================
    UpdateGame(deltaTime);
    
    //=====================================
    // 物理計算
    //=====================================
    mPhysWorld->Test();
    
    //=====================================
    // Actor 更新
    //=====================================
    mIsUpdatingActors = true;
    for (auto& a : mActors)
    {
        a->Update(deltaTime);
    }
    mIsUpdatingActors = false;
    
    // Pending にある Actor を本体リストへ移動
    for (auto& p : mPendingActors)
    {
        p->ComputeWorldTransform();
        mActors.emplace_back(std::move(p));
    }
    mPendingActors.clear();
    
    // EDead フラグの Actor を削除
    mActors.erase(
        std::remove_if(
            mActors.begin(),
            mActors.end(),
            [](const std::unique_ptr<Actor>& actor)
            {
                return actor->GetState() == Actor::EDead;
            }
        ),
        mActors.end()
    );
    
    //=====================================
    // サウンド更新（リスナー位置はカメラの逆行列から取得）
    //=====================================
    if (mSoundMixer)
    {
        Matrix4 inv = GetRenderer()->GetInvViewMatrix();
        mSoundMixer->Update(deltaTime, inv);
    }
}


//=============================================================
// アセットディレクトリの設定
//=============================================================

void Application::InitAssetManager(const std::string& path, float dpi)
{
    mAssetManager->SetAssetsPath(path);
    mAssetManager->SetWindowDisplayScale(dpi);
}

} // namespace toy
