
// ParticleComponent.cpp
#include "ParticleComponent.h"
#include "Actor.h"
#include "Shader.h"
#include "Texture.h"
#include "Application.h"
#include "Renderer.h"
#include "VertexArray.h"
#include <random>

ParticleComponent::ParticleComponent(Actor* owner, int drawOrder)
    : VisualComponent(owner, drawOrder)
    , mTexture(nullptr)
    , mDrawOrder(drawOrder)
    , mIsBlendAdd(true)
    , mNumParts(0)
    , mLifeTime(0.0f)
    , mTotalLife(0.0f)
    , mPartLifecycle(0.0f)
    , mPartSize(0.0f)
    , mPartSpeed(2.0f)
    , mParticleMode(P_SPARK)
{
    mLayer = VisualLayer::Effect3D;
    mShader = mOwnerActor->GetApp()->GetRenderer()->GetShader("Particle");
}

ParticleComponent::~ParticleComponent()
{
    mParts.clear();
}

void ParticleComponent::SetTexture(std::shared_ptr<Texture> tex)
{
    mTexture = tex;
}

void ParticleComponent::CreateParticles(Vector3 pos, unsigned int num, float life, float partLife, float size, ParticleMode mode)
{
    mPosition = pos;
    mIsVisible = true;
    mNumParts = num;
    mLifeTime = 0.0f;
    mTotalLife = life;
    mPartLifecycle = partLife;
    mPartSize = size;
    mParticleMode = mode;

    mParts.resize(mNumParts);
}

void ParticleComponent::GenerateParts()
{
    std::random_device rnd;

    for (int i = 0; i < mNumParts; ++i)
    {
        if (mParts[i].isVisible) continue;
     
        float x = (float)(rnd() % (int)mPartSpeed);
        float y = (float)(rnd() % (int)mPartSpeed);
        float z = (float)(rnd() % (int)mPartSpeed);
        if (rand() % 2) x *= -1;
        if (rand() % 2) y *= -1;
        if (rand() % 2) z *= -1;

        mParts[i].pos = mPosition;
        mParts[i].dir = Vector3(x, y, z);
        mParts[i].isVisible = true;
        mParts[i].lifeTime = 0.0f;
        mParts[i].size = mPartSize;
        break;
    }
}

void ParticleComponent::Update(float deltaTime)
{
    mLifeTime += deltaTime;
    if (mLifeTime > mTotalLife)
    {
        mIsVisible = false;
    }

    for (int i = 0; i < mNumParts; ++i)
    {
        if (mParts[i].isVisible)
        {
            if (mParticleMode == P_WATER)
                mParts[i].dir.y -= 0.04f;
            else if (mParticleMode == P_SMOKE)
                mParts[i].dir.y += 0.04f;

            mParts[i].lifeTime += deltaTime;
            mParts[i].pos += mParts[i].dir * deltaTime;

            if (mParts[i].lifeTime > mPartLifecycle)
                mParts[i].isVisible = false;
        }
    }

    if (rand() % 2 == 0)
    {
        GenerateParts();
    }
}

void ParticleComponent::Draw()
{
    if (!mIsVisible || mTexture == nullptr) return;

    if (mIsBlendAdd)
    {
        glBlendFunc(GL_ONE, GL_ONE);
    }
    else
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    Matrix4 mat = mOwnerActor->GetWorldTransform();
    Matrix4 invView = mOwnerActor->GetApp()->GetRenderer()->GetInvViewMatrix();
    invView.mat[3][0] = mat.mat[3][0];
    invView.mat[3][1] = mat.mat[3][1];
    invView.mat[3][2] = mat.mat[3][2];

    Matrix4 scaleMat = Matrix4::CreateScale(mPartSize, mPartSize, 1);
    Matrix4 world = scaleMat * Matrix4::CreateScale(mOwnerActor->GetScale()) * invView;

    auto renderer = mOwnerActor->GetApp()->GetRenderer();
    Matrix4 view = renderer->GetViewMatrix();
    Matrix4 proj = renderer->GetProjectionMatrix();
    
    mShader->SetActive();
    mShader->SetMatrixUniform("uViewProj", view * proj);
    mShader->SetMatrixUniform("uWorldTransform", world);

    mTexture->SetActive(0);
    mShader->SetTextureUniform("uTexture", 0);

    // VAO有効有効化
    mVertexArray->SetActive();
    for (int i = 0; i < mNumParts; ++i)
    {
        if (mParts[i].isVisible)
        {
            mShader->SetVectorUniform("uPosition", mParts[i].pos);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }
    }
    if (mIsBlendAdd)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}
