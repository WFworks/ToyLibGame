#include "LightingManager.h"
#include "Shader.h"

LightingManager::LightingManager()
{
    
}

LightingManager::~LightingManager()
{
    
}


void LightingManager::ApplyToShader(std::shared_ptr<Shader> shader, const Matrix4& viewMatrix)
{
    // カメラポジション（ビュー行列の逆行列から取得）
    Matrix4 invView = viewMatrix;
    invView.Invert();
    shader->SetVectorUniform("uCameraPos", invView.GetTranslation());

    // アンビエント
    shader->SetVectorUniform("uAmbientLight", mAmbientColor);

    // ライト方向を再計算（Target - Position）
    shader->SetVectorUniform("uDirLight.mDirection", mDirLight.GetDirection());
    shader->SetVectorUniform("uDirLight.mDiffuseColor", mDirLight.DiffuseColor);
    shader->SetVectorUniform("uDirLight.mSpecColor", mDirLight.SpecColor);

    // フォグ情報
    shader->SetFloatUniform("uFoginfo.maxDist", mFog.MaxDist);
    shader->SetFloatUniform("uFoginfo.minDist", mFog.MinDist);
    shader->SetVectorUniform("uFoginfo.color", mFog.Color);
}
