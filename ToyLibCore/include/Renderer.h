#pragma once

#include "MathUtils.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <GL/glew.h>

enum class VisualLayer
{
    Background2D,
    Effect3D,
    Object3D,
    OverlayScreen,
    UI,
};

// 描画エンジン
class Renderer
{
public:
    // コンストラクタ
    Renderer();
    // デストラクタ
    virtual ~Renderer();
    // 初期化
    bool Initialize();
    SDL_Window* GetSDLWindow() const { return mWindow; }
    // 描画（Applicationから呼ばれる）
    void Draw();
    // 終了
    void Shutdown();
    // クリア色を設定
    void SetClearColor(const Vector3 color);

    // ビューマトリックスを設定する
    void SetViewMatrix(const Matrix4& view) { mInvView = mViewMatrix = view; mInvView.Invert();}
    Matrix4 GetViewMatrix() const { return mViewMatrix; }
    Matrix4 GetInvViewMatrix() const { return mInvView; }
    Matrix4 GetProjectionMatrix() const { return mProjectionMatrix; }
    Matrix4 GetViewProjMatrix() const { return mViewMatrix * mProjectionMatrix; }
    
    // FOV取得（度）
    float GetPerspectiveFov() const { return mPerspectiveFOV; }
    void SetPerspectiveFov(float f) { mPerspectiveFOV = f; }
    
    // スクリーンサイズのGtter
    float GetScreenWidth() const { return mScreenWidth; }
    float GetScreenHeight() const { return mScreenHeight; }
      
    
    // Visualコンポーネント
    void AddVisualComp(class VisualComponent* comp);
    void RemoveVisualComp(class VisualComponent* comp);

    // デバッグモード設定
    void SetDebugMode(const bool b) { mIsDebugMode = b; }
    bool GetDebugMode() const { return mIsDebugMode; }
    

    // データ解放
    void UnloadData();
    
    // スカイドーム登録
    void RegisterSkyDome(class SkyDomeComponent* sky);
    class SkyDomeComponent* GetSkyDome() const { return mSkyDomeComp; }
    
    
    // ライトマネージャー
    void SetLightingManager(std::shared_ptr<class LightingManager> manager) { mLightingManager = manager; }
    std::shared_ptr<class LightingManager> GetLightingManager() const { return mLightingManager; }
    
    
    std::shared_ptr<class Shader> GetShader(const std::string& name) { return mShaders[name]; }
    
    // 光源マトリックス
    Matrix4 GetLightSpaceMatrix() const { return mLightSpaceMatrix; }
    
    // スプライト用VAO取得
    std::shared_ptr<class VertexArray> GetSpriteVerts() const { return mSpriteVerts; }
    // フルスクリーン用VAO取得
    std::shared_ptr<class VertexArray> GetFullScreenQuad() const { return mFullScreenQuad; }
    
    std::shared_ptr<class Texture> GetShadowMapTexture() const { return mShadowMapTexture; }
    
    bool IsDebugMode() const { return mIsDebugMode; }

private:
    // セッティング読み込み
    bool LoadSettings(const std::string& filePath);
    // ライティング管理
    std::shared_ptr<class LightingManager> mLightingManager;
    
    // レンダラーパラメーター //
    std::string mShaderPath;
    // ウィンドウタイトル
    std::string mStrTitle;
    //スクリーンサイズ
    float mScreenWidth;
    float mScreenHeight;
    bool mIsFullScreen;
    // 視野角(度)
    float mPerspectiveFOV;
    // デバッグモード
    bool mIsDebugMode;
    // クリア色
    Vector3 mClearColor;

    // シャドウマップ
    float mShadowNear;
    float mShadowFar;
    float mShadowOrthoWidth;
    float mShadowOrthoHeight;
    int mShadowFBOWidth;
    int mShadowFBOHeight;
    

    // ビューマトリックス
    Matrix4 mViewMatrix;
    Matrix4 mInvView;
    // プロジェクションマトリックス
    Matrix4 mProjectionMatrix;

    // Windowハンドラ
    SDL_Window* mWindow;
    // GLコンテキスト
    SDL_GLContext mGLContext;
    
    // フルスクリーンVAO
    std::shared_ptr<class VertexArray> mFullScreenQuad;
    void CreateFullScreenQuad();
    
   
    // シェーダー格納
    std::unordered_map<std::string, std::shared_ptr<class Shader>> mShaders;
    // シェーダー一括ロード
    bool LoadShaders();

    // スプライト用ポリゴン（Billboardでも使う）
    std::shared_ptr<class VertexArray> mSpriteVerts;
    // スプライト用ポリゴンの生成
    void CreateSpriteVerts();
    
    // シャドウマッピング関連処理とパラメータ
    GLuint mShadowFBO;
    bool InitializeShadowMapping();
    void RenderShadowMap();
    Matrix4 mLightSpaceMatrix;
    std::shared_ptr<class Texture> mShadowMapTexture;

    // コンポーネント
    std::vector<class VisualComponent*> mVisualComps;
    class SkyDomeComponent* mSkyDomeComp; // Gameアプリ側で生成、生ポインタを保持
    
    void DrawSky();
    void DrawVisualLayer(VisualLayer layer);
};

