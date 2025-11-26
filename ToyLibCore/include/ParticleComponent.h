// ParticleComponent.h
#pragma once
#include "VisualComponent.h"
#include <vector>

class ParticleComponent : public VisualComponent
{
public:
    enum ParticleMode
    {
        P_SPARK,
        P_WATER,
        P_SMOKE
    };

    struct Particle
    {
        Vector3 pos;
        Vector3 dir;
        float lifeTime;
        float size;
        bool isVisible;
    };

    ParticleComponent(class Actor* owner, int drawOrder = 20);
    ~ParticleComponent();

    void Update(float deltaTime) override;
    void Draw() override;

    void SetTexture(std::shared_ptr<class Texture> tex) override;
    void CreateParticles(Vector3 pos, unsigned int num, float life, float partLife, float size, ParticleMode mode);

    void SetAddBlend(bool b) { mIsBlendAdd = b; }
    void SetSpeed(float speed) { mPartSpeed = speed; }

    int GetDrawOrder() const { return mDrawOrder; }

private:
    void GenerateParts();

    std::shared_ptr<class Texture> mTexture;
    Vector3 mPosition;
    std::vector<Particle> mParts;


    int mDrawOrder;
    unsigned int mNumParts;
    float mLifeTime;
    float mTotalLife;
    float mPartLifecycle;
    float mPartSize;
    float mPartSpeed;

    ParticleMode mParticleMode;
    bool mIsBlendAdd;
};
