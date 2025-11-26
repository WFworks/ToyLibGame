#include "Material.h"
#include "Shader.h"
#include "Texture.h"

Material::Material()
    : mAmbientColor(0.2f, 0.2f, 0.2f)
    , mDiffuseColor(0.8f, 0.8f, 0.8f)
    , mSpecularColor(1.0f, 1.0f, 1.0f)
    , mShininess(32.0f)
    , mDiffuseMap(nullptr)
    , mOverrideColor(false)
    , mUniformColor(Vector3::Zero)
{
}

void Material::BindToShader(std::shared_ptr<Shader> shader, int textureUnit) const
{
    shader->SetBooleanUniform("uOverrideColor", mOverrideColor);
    shader->SetVectorUniform("uUniformColor", mUniformColor);

    // カラー情報
    shader->SetVectorUniform("uAmbientColor", mAmbientColor);
    shader->SetVectorUniform("uDiffuseColor", mDiffuseColor);
    shader->SetVectorUniform("uSpecColor", mSpecularColor);
    shader->SetFloatUniform("uSpecPower", mShininess);

    // テクスチャ設定（基本は1枚のみ）
    if (mDiffuseMap)
    {
        mDiffuseMap->SetActive(textureUnit);
        shader->SetTextureUniform("uTexture", textureUnit);
    }
}

void Material::SetOverrideColor(bool enable, const Vector3& color)
{
    mOverrideColor = enable;
    mUniformColor = color;
}
