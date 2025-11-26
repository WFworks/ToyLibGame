#include "Application.h"
#include "Actor.h"
#include "Renderer.h"
#include "InputSystem.h"
#include "PhysWorld.h"
#include "IMEUtil.h"
#include "AssetManager.h"
#include "SoundMixer.h"

#include <algorithm>
#include <SDL2/SDL_syswm.h>


Application::Application()
: mIsActive(false)
, mIsUpdatingActors(false)
, mIsPause(false)
, mNativeWindowHandle(nullptr)
{
    mRenderer = std::make_unique<Renderer>();
    mInputSys = std::make_unique<InputSystem>();
    mPhysWorld = std::make_unique<PhysWorld>();
    mAssetManager = std::make_unique<AssetManager>();
    mSoundMixer = std::make_unique<SoundMixer>(mAssetManager.get());
    
}

// デストラクタ
Application::~Application()
{
    
}


// アプリ初期化
bool Application::Initialize()
{
    
    // SDL初期化
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Failed to init SDL\n";
        return false;
    }
    
    // Renderer初期化
    mRenderer->Initialize();

    // 入力システム初期化
    mInputSys->Initialize();
    
    // データ ロード、主にRendererに登録されるもの。
    LoadData();
    
    // 派生先のInitを呼ぶ
    InitGame();
    
    mIsActive = true;
    mTicksCount = SDL_GetTicks();
    
    
    // IME制御
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(GetRenderer()->GetSDLWindow(), &wmInfo))
    {
#if defined(_WIN32)
        mNativeWindowHandle = static_cast<void*>(wmInfo.info.win.window);
#else
        mNativeWindowHandle = nullptr;
#endif
    }

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

// 描画処理 Rendererが描画する
void Application::Draw()
{
    mRenderer->Draw();
}


// 終了処理
void Application::Shutdown()
{
    ShutdownGame();
    UnloadData();
    mInputSys->Shutdown();
    mRenderer->Shutdown();
    SDL_Quit();
}

// 入力受付
void Application::ProcessInput(){
    
    // 前の状態を保存
    mInputSys->PrepareForUpdate();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
                
        case SDL_QUIT:
            mIsActive = false;
            break;
        }
    }

    mInputSys->Update();
    const InputState& state = mInputSys->GetState();
        
    if (state.Keyboard.GetKeyState(SDL_SCANCODE_ESCAPE) == EReleased)
    {
        mIsActive = false;
    }

    if (state.Keyboard.GetKeyState(SDL_SCANCODE_SPACE) == EHeld)
    {
        mIsPause = true;
    }
    else
    {
        mIsPause = false;
    }
    
    // 配下のActorsに渡す
    for (auto& actor : mActors)
    {
        actor->ProcessInput(state);
    }
}


// Actor追加
void Application::AddActor(std::unique_ptr<Actor> actor)
{
    if (mIsUpdatingActors)
    {
        mPendingActors.emplace_back(std::move(actor));
    }
    else
    {
        mActors.emplace_back(std::move(actor));
    }
}


// データ解放
void Application::UnloadData()
{
    mActors.clear();

    if (mRenderer)
    {
        mRenderer->UnloadData();
    }
}

// Actors, Renderer関連
void Application::LoadData()
{
    // ライト
    //mRenderer->SetAmbientColor(Vector3(0.9f, 0.9f, 0.9f));
    //DirectionalLight& dir = mRenderer->GetDirectionalLight();
    //dir.Direction = Vector3(0.f, 0.f, -1.f);
    //dir.DiffuseColor = Vector3(0.4f, 0.4f, 0.4f);
    //dir.SpecColor = Vector3(0.1f, 0.1f, 0.1f);
    

}

// ゲームメインルーチン
void Application::UpdateFrame()
{
    
    // FPS60固定
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
        ;

    float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }
    mTicksCount = SDL_GetTicks();

    // ポーズ中以降の処理キャンセル
    if(mIsPause) return;
    
    // 派生先のUpdateを呼ぶ
    UpdateGame(deltaTime);
    
    // 物理計算系
    //physWorld->ComputeGroundHeight();
    mPhysWorld->Test();
    

    // Actorsメイン呼び出し
    mIsUpdatingActors = true;
    for (auto& a : mActors)
    {
        a->Update(deltaTime);
    }
    mIsUpdatingActors = false;

    // Pendingがある場合はActorsに移動
    for (auto& p : mPendingActors)
    {
        p->ComputeWorldTransform();
        mActors.emplace_back(std::move(p));
    }
    mPendingActors.clear();

    // EDeadフラグのアクターは削除
    mActors.erase(std::remove_if(mActors.begin(), mActors.end(),
                                 [](const std::unique_ptr<Actor>& actor) {
                                    return actor->GetState() == Actor::EDead;
                                }),mActors.end());
}

// IME制御
void Application::SetIMEEnabled(bool enabled)
{
    IMEUtil::SetIMEEnabled(mNativeWindowHandle, enabled);
}

// アセットディレクトリの設定
void Application::SetAssetsPath(const std::string &path)
{
    mAssetManager->SetAssetsPath(path);
}
