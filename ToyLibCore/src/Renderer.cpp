#include "Renderer.h"
#include "Shader.h"
#include "LightingManager.h"
#include "SpriteComponent.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "SkeletalMeshComponent.h"
#include "ParticleComponent.h"
#include "BillboardComponent.h"
#include "VisualComponent.h"
#include "SkyDomeComponent.h"
#include "WireframeComponent.h"
#include <GL/glew.h>
#include <algorithm>
#include <string>

// コンストラクタ
Renderer::Renderer()
: mStrTitle("ToyLib App")
, mScreenWidth(0.f)
, mScreenHeight(0.f)
, mIsFullScreen(false)
, mPerspectiveFOV(45.f)
, mIsDebugMode(false)
, mClearColor(Vector3(0.2f, 0.5f, 0.8f))
, mShadowNear(10.f)
, mShadowFar(100)
, mShadowOrthoWidth(100.f)
, mShadowOrthoHeight(100.f)
, mShadowFBOWidth(4096)
, mShadowFBOHeight(4096)
, mWindow(nullptr)
, mGLContext(nullptr)
, mShaderPath("ToyLibCore/Shaders/")
{
    LoadSettings("ToyLibCore/Settings/Renderer_Settings.json");
}

// デストラクタ
Renderer::~Renderer()
{
}

// ウィンドウ生成とGL初期化
bool Renderer::Initialize()
{
    // OpenGL プロファイル, バージョン
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // ダブルバッファリング
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // RGBAチャンネル
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    // ハードウェアアクセラレーション有効化
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    unsigned int WINDOW_FLAGS = SDL_WINDOW_OPENGL;
    if (mIsFullScreen)
    {
        WINDOW_FLAGS =  WINDOW_FLAGS | SDL_WINDOW_FULLSCREEN;
    }

    //ウインドウ生成
    mWindow = SDL_CreateWindow(mStrTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, static_cast<int>(mScreenWidth), static_cast<int>(mScreenHeight), WINDOW_FLAGS);
    
    if (!mWindow )
    {
        std::cout << "Unable to create window" << std::endl;
        return false;
    }
    // OpenGL コンテキスト生成
    mGLContext = SDL_GL_CreateContext(mWindow);
    
    // GLEW初期化
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return false;
    }
   
    // ライティングマネージャ生成
    mLightingManager = std::make_shared<LightingManager>();
    // シェーダー のロードなどはここでやる
    LoadShaders();
    // スプライト用頂点バッファ
    CreateSpriteVerts();
    // オーバーレイエフェクト用頂点バッファ
    CreateFullScreenQuad();
    // シャドウマッピング初期化
    InitializeShadowMapping();
    // クリアカラーのデフォルト値
    SetClearColor(mClearColor);

    return true;
}

// リリース処理
void Renderer::Shutdown()
{
    SDL_GL_DeleteContext(mGLContext);
    SDL_DestroyWindow(mWindow);

}

// 描画処理
void Renderer::Draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderShadowMap();
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DrawSky();
    DrawVisualLayer(VisualLayer::Background2D);
    DrawVisualLayer(VisualLayer::Object3D);
    DrawVisualLayer(VisualLayer::Effect3D);
    DrawVisualLayer(VisualLayer::OverlayScreen);

    //DrawWeatherOverlay();
    
    DrawVisualLayer(VisualLayer::UI);

    SDL_GL_SwapWindow(mWindow);
}

void Renderer::DrawSky()
{
    if (!mSkyDomeComp) return;
    mSkyDomeComp->Draw();

}

void Renderer::AddVisualComp(VisualComponent* comp)
{
    auto iter = mVisualComps.begin();
    for (; iter != mVisualComps.end(); ++iter)
    {
        if (comp->GetDrawOrder() < (*iter)->GetDrawOrder())
            break;
    }
    mVisualComps.insert(iter, comp);
}

void Renderer::RemoveVisualComp(VisualComponent* comp)
{
    auto iter = std::find(mVisualComps.begin(), mVisualComps.end(), comp);
    if (iter != mVisualComps.end())
        mVisualComps.erase(iter);
}

void Renderer::DrawVisualLayer(VisualLayer layer)
{
    if (layer == VisualLayer::UI || layer == VisualLayer::Background2D)
    {
        glDisable(GL_DEPTH_TEST);     // Zテスト不要
        glDepthMask(GL_FALSE);        // 書き込みも不要（2D要素）
    }
    else if (layer == VisualLayer::Effect3D)
    {
        glEnable(GL_DEPTH_TEST);     // 粒同士のZ隠し合いを防ぐ
        glDepthMask(GL_FALSE);        // Zバッファ汚さない
    }
    else
    {
        glEnable(GL_DEPTH_TEST);      // 通常描画
        glDepthMask(GL_TRUE);         // 書き込みON
    }
    
    
    for (auto comp : mVisualComps)
    {
        if (comp->IsVisible() && comp->GetLayer() == layer)
        {
            comp->Draw();
        }
    }

    glEnable(GL_DEPTH_TEST); // 念のため戻す
    glDepthMask(GL_TRUE);
}


//スプライト用ポリゴン
void Renderer::CreateSpriteVerts()
{
    const float vertices[] =
    {
        -0.5f, 0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 0.f, // top left
        0.5f, 0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 0.f, // top right
        0.5f,-0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 1.f, // bottom right
        -0.5f,-0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 1.f  // bottom left
    };
    

    const unsigned int indices[] =
    {
        2, 1, 0,
        0, 3, 2
    };
    mSpriteVerts = std::make_shared<VertexArray>((float*)vertices, (unsigned int)4, (unsigned int*)indices, (unsigned int)6);
}


// データ解放
void Renderer::UnloadData()
{

    mVisualComps.clear();
}

// シャドウマッピング
bool Renderer::InitializeShadowMapping()
{

    // シャドウマップ用のFBO作成
    glGenFramebuffers(1, &mShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);

    // シャドウ用テクスチャ生成
    mShadowMapTexture = std::make_shared<Texture>();
    mShadowMapTexture->CreateShadowMap(mShadowFBOWidth, mShadowFBOHeight);

    // FBOにアタッチ
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowMapTexture->GetTextureID(), 0);
/*
    // 確認用
    GLint compareMode;
    glBindTexture(GL_TEXTURE_2D, mShadowMapTexture->GetTextureID());
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, &compareMode);
    SDL_Log("Compare mode: %d", compareMode); // GL_COMPARE_REF_TO_TEXTUREでなければNG（値は 34894）
  */
    // カラーバッファなし（深度のみ）
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // 完成チェック
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error: Shadow framebuffer is not complete!" << std::endl;
        return false;
    }

    // FBOのバインド解除
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

// シャドウマップのレンダリング
void Renderer::RenderShadowMap()
{
    // FBOバインドして深度バッファだけ描画
    glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);
    glViewport(0, 0, static_cast<GLsizei>(mShadowFBOWidth), static_cast<GLsizei>(mShadowFBOHeight));
    
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    // ビュー・プロジェクション行列
    // カメラ追従
    Vector3 camCenter = mInvView.GetTranslation() + mInvView.GetZAxis() * 30.0f;
    //Vector3 lightDir = Vector3::Normalize(mDirLight.Direction);
    Vector3 lightDir = mLightingManager->GetLightDirection();
    Vector3 lightPos = camCenter - lightDir * 50.0f;
    
    Matrix4 lightViewMatrix = Matrix4::CreateLookAt(lightPos, camCenter, Vector3::UnitY);
    Matrix4 lightProjMatrix = Matrix4::CreateOrtho(mShadowOrthoWidth, mShadowOrthoHeight, mShadowNear, mShadowFar);
    mLightSpaceMatrix =  lightViewMatrix * lightProjMatrix;

    for (auto& visual : mVisualComps)
    {
        if (visual->IsEnableShadow() && visual->IsVisible())
        {
            visual->DrawShadow();
        }
    }

    // ビューポートを戻す（スクリーンサイズ）
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, static_cast<GLsizei>(mScreenWidth), static_cast<GLsizei>(mScreenHeight));
}

// 雨エフェクトの初期化
void Renderer::CreateFullScreenQuad()
{
    // クアッド作成
    float quadVerts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };
    mFullScreenQuad = std::make_shared<VertexArray>(quadVerts, 4, quadIndices, 6, true);

}

void Renderer::RegisterSkyDome(SkyDomeComponent* sky)
{
    mSkyDomeComp = sky;
    if (mSkyDomeComp)
    {
        mSkyDomeComp->SetLightingManager(mLightingManager);
    }
}

void::Renderer::SetClearColor(const Vector3 color)
{
    mClearColor = color;
    glClearColor(mClearColor.x, mClearColor.y, mClearColor.z, 1.0f);
}


// シェーダー 読み込み
bool Renderer::LoadShaders()
{
    std::string vShaderName;
    std::string fShaderName;

    
    // 天気シェーダー（統合版）
    vShaderName = mShaderPath + "WeatherScreen.vert";
    fShaderName = mShaderPath + "WeatherScreen.frag";
    mShaders["WeatherOverlay"] = std::make_shared<Shader>();
    if (!mShaders["WeatherOverlay"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // メッシュ用シェーダー
    vShaderName = mShaderPath + "Phong.vert";
    fShaderName = mShaderPath + "Phong.frag";
    mShaders["Mesh"] = std::make_shared<Shader>();
    if (!mShaders["Mesh"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // スキンメッシュ用シェーダー
    vShaderName = mShaderPath + "Skinned.vert";
    fShaderName = mShaderPath + "Phong.frag";
    mShaders["Skinned"] = std::make_shared<Shader>();
    if (!mShaders["Skinned"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // スプライト用シェーダー
    vShaderName = mShaderPath +"Sprite.vert";
    fShaderName = mShaderPath + "Sprite.frag";
    mShaders["Sprite"] = std::make_shared<Shader>();
    if (!mShaders["Sprite"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    Matrix4 viewProj = Matrix4::CreateSimpleViewProj(mScreenWidth, mScreenHeight);
    mShaders["Sprite"]->SetMatrixUniform("uViewProj", viewProj);
    // ビルボード用シェーダー
    vShaderName = mShaderPath + "Billboard.vert";
    fShaderName = mShaderPath + "Billboard.frag";
    mShaders["Billboard"] = std::make_shared<Shader>();
    if (!mShaders["Billboard"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // パーティクル
    vShaderName = mShaderPath + "Billboard.vert";
    fShaderName = mShaderPath + "Particle.frag";
    mShaders["Particle"] = std::make_shared<Shader>();
    if (!mShaders["Particle"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // ソリッドカラー
    vShaderName = mShaderPath + "BasicMesh.vert";
    fShaderName = mShaderPath + "SolidColor.frag";
    mShaders["Solid"] = std::make_shared<Shader>();
    if (!mShaders["Solid"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // シャドウマップ用（Skinned）
    vShaderName = mShaderPath + "ShadowMapping_Skinned.vert";
    fShaderName = mShaderPath + "ShadowMapping.frag";
    mShaders["ShadowSkinned"] = std::make_shared<Shader>();
    if (!mShaders["ShadowSkinned"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // シャドウマップ用（Mesh）
    vShaderName = mShaderPath + "ShadowMapping_Mesh.vert";
    fShaderName = mShaderPath + "ShadowMapping.frag";
    mShaders["ShadowMesh"] = std::make_shared<Shader>();
    if (!mShaders["ShadowMesh"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // スカイドーム
    vShaderName = mShaderPath + "WeatherDome.vert";
    fShaderName = mShaderPath + "WeatherDome.frag";
    mShaders["SkyDome"] = std::make_shared<Shader>();
    if (!mShaders["SkyDome"]->Load(vShaderName.c_str(), fShaderName.c_str()))
    {
        return false;
    }
    // ビューマトリックス、プロジェクションマトリックス（デフォルト値）
    mViewMatrix = Matrix4::CreateLookAt(Vector3(0, 0.5f, -3), Vector3(0, 0, 10), Vector3::UnitY);
    mProjectionMatrix = Matrix4::CreatePerspectiveFOV(Math::ToRadians(mPerspectiveFOV), mScreenWidth, mScreenHeight, 1.0f, 2000.0f);

    return true;
}
