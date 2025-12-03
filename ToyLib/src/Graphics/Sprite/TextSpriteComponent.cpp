#include "Graphics/Sprite/TextSpriteComponent.h"
#include "Asset/Font/TextFont.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Application.h"
#include "Engine/Render/Renderer.h"
#include "Asset/Material/Texture.h"

namespace toy {

TextSpriteComponent::TextSpriteComponent(Actor* owner, int drawOrder, VisualLayer layer)
: SpriteComponent(owner, drawOrder, layer)
, mText("")
, mColor(1.0f, 1.0f, 1.0f)
, mFont(nullptr)
{
}

TextSpriteComponent::~TextSpriteComponent()
{

}

void TextSpriteComponent::SetText(const std::string& text)
{
    if (mText == text)
    {
        return;
    }
    mText = text;
    UpdateTexture();
}

void TextSpriteComponent::SetColor(const Vector3& color)
{
    mColor = color;
    UpdateTexture();
}

void TextSpriteComponent::SetFont(std::shared_ptr<TextFont> font)
{
    mFont = font;
    UpdateTexture();
}

void TextSpriteComponent::Refresh()
{
    UpdateTexture();
}

void TextSpriteComponent::UpdateTexture()
{
    // 前提条件が揃ってないならテクスチャをクリア
    if (mText.empty() || !mFont || !mFont->IsValid())
    {
        SetTexture(nullptr);
        return;
    }

    auto* app = GetOwner()->GetApp();
    auto* renderer = app->GetRenderer();

    auto tex = renderer->CreateTextTexture(mText, mColor, mFont);
    if (!tex)
    {
        // テキスト生成に失敗した場合は消しておく
        SetTexture(nullptr);
        return;
    }

    SetTexture(tex);
    // サイズは SpriteComponent 側で tex->GetWidth/Height を使ってくれるので
    // ここでは特にやることなし（スケーリングしたいときは SetScale() を別途呼ぶ）
}

} // namespace toy
