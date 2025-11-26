#pragma once
#include "MathUtils.h"
#include <memory>

class Material
{
public:
    Material();
    void BindToShader(std::shared_ptr<class Shader> shader, int textureUnit = 0) const;

    void SetDiffuseMap(std::shared_ptr<class Texture> tex) { mDiffuseMap = tex; }

    void SetSpecPower(float power) { mShininess = power; }
    void SetOverrideColor(bool enable, const Vector3& color);

    void SetDiffuseColor(const Vector3& color) { mDiffuseColor = color; }
    void SetSpecularColor(const Vector3& color) { mSpecularColor = color; }
    void SetAmbientColor(const Vector3& color) { mAmbientColor = color; }

private:
    std::shared_ptr<class Texture> mDiffuseMap;

    float mShininess = 32.0f;

    Vector3 mAmbientColor;
    Vector3 mDiffuseColor;
    Vector3 mSpecularColor;

    bool mOverrideColor = false;
    Vector3 mUniformColor = Vector3::Zero;
};
