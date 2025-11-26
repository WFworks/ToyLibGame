#include "WeatherOverlayComponent.h"
#include "Actor.h"
#include "Application.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "MathUtils.h"


WeatherOverlayComponent::WeatherOverlayComponent(Actor* a, int drawOrder, VisualLayer layer)
: VisualComponent(a, drawOrder, layer)
, mRainAmount(0.f)
, mFogAmount(0.f)
, mSnowAmount(0.f)
{
    auto renderer = mOwnerActor->GetApp()->GetRenderer();
    mShader = renderer->GetShader("WeatherOverlay");
    mVertexArray = renderer->GetFullScreenQuad();
    mScreenWidth = renderer->GetScreenWidth();
    mScreenHeight = renderer->GetScreenHeight();
}

void WeatherOverlayComponent::Draw()
{
    if (!mShader || !mVertexArray) return;

 
    // フルスクリーン用設定
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // シェーダーをアクティブ化
    mShader->SetActive();

    // ユニフォーム設定
    mShader->SetFloatUniform("uTime", SDL_GetTicks() / 1000.0f); // 経過時間（秒）
    mShader->SetFloatUniform("uRainAmount", mRainAmount); // 0〜1で設定
    mShader->SetFloatUniform("uFogAmount", mFogAmount); // 0〜1で設定
    mShader->SetFloatUniform("uSnowAmount", mSnowAmount); // 0〜1で設定
    mShader->SetVector2Uniform("uResolution", Vector2(mScreenWidth, mScreenHeight));

    // フルスクリーンポリゴンを描画
    mVertexArray->SetActive();
    glDrawElements(GL_TRIANGLES, mVertexArray->GetNumIndices(), GL_UNSIGNED_INT, nullptr);

    // 状態戻す
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}
