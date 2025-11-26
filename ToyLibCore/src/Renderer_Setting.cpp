#include "Renderer.h"
#include "JsonHelper.h"
#include <fstream>


bool Renderer::LoadSettings(const std::string& filePath)
{
    
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        SDL_Log("Failed to open settings file: %s", filePath.c_str());
        return false;
    }

    nlohmann::json data;
    try
    {
        file >> data;
    }
    catch (const std::exception& e)
    {
        SDL_Log("JSON parse error: %s", e.what());
        return false;
    }

    // タイトル
    JsonHelper::GetString(data, "title", mStrTitle);
    
    // シェーダーパス
    JsonHelper::GetString(data, "shader_path", mShaderPath);

    // 画面サイズ
    if (data.contains("screen"))
    {
        JsonHelper::GetFloat(data["screen"], "width", mScreenWidth);
        JsonHelper::GetFloat(data["screen"], "height", mScreenHeight);
        JsonHelper::GetBool(data["screen"], "fullscreen", mIsFullScreen);
    }

    // FOV
    JsonHelper::GetFloat(data, "perspectiveFOV", mPerspectiveFOV);

    // カメラ位置
    //JsonHelper::GetVector3(data["camera"], "position", mCameraPosition);

    // デバッグモード
    if (data.contains("debug"))
    {
        JsonHelper::GetBool(data["debug"], "enabled", mIsDebugMode);
    }

    // クリアカラー
    JsonHelper::GetVector3(data, "clearColor", mClearColor);

    // ライト設定
    //JsonHelper::GetVector3(data, "ambient", mAmbientColor);
    //JsonHelper::GetVector3(data, "specular", mSpecColor);

    /*
    if (data.contains("directionalLight"))
    {
        JsonHelper::GetVector3(data["directionalLight"], "diffuse", mDiffuseColor);
        JsonHelper::GetVector3(data["directionalLight"], "position", mDirLightPosition);
        JsonHelper::GetVector3(data["directionalLight"], "target", mDirLightTarget);
    }

    // フォグ設定
    if (data.contains("fog"))
    {
        JsonHelper::GetFloat(data["fog"], "maxDist", mFogMaxDist);
        JsonHelper::GetFloat(data["fog"], "minDist", mFogMinDist);
        JsonHelper::GetVector3(data["fog"], "color", mFogColor);
    }
    */
    // シャドウ設定
    if (data.contains("shadow"))
    {
        JsonHelper::GetFloat(data["shadow"], "near", mShadowNear);
        JsonHelper::GetFloat(data["shadow"], "far", mShadowFar);
        JsonHelper::GetFloat(data["shadow"], "ortho_width", mShadowOrthoWidth);
        JsonHelper::GetFloat(data["shadow"], "ortho_height", mShadowOrthoHeight);
        JsonHelper::GetInt(data["shadow"], "resolution_width", mShadowFBOWidth);
        JsonHelper::GetInt(data["shadow"], "resolution_height", mShadowFBOHeight);
    }

    SDL_Log("Loaded Renderer settings from %s", filePath.c_str());
    return true;
}
