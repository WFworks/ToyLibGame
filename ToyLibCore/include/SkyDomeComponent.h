#pragma once
#include "Component.h"
#include "Shader.h"
#include <memory>

class SkyDomeComponent : public Component
{
public:
    SkyDomeComponent(class Actor* a);

    virtual void Draw();
    void Update(float deltaTime) override;
    

    void SetLightingManager(std::shared_ptr<class LightingManager> manager) { mLightingManager = manager; }
    
protected:
    std::unique_ptr<class VertexArray> mSkyVAO;
    std::shared_ptr<class LightingManager> mLightingManager;
    std::shared_ptr<class Shader> mShader;
};
