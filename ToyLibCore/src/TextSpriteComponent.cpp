#include "TextSpriteComponent.h"
#include "Font.h"
#include "Actor.h"
#include "Application.h"
#include "Renderer.h"
#include "Texture.h"

TextSpriteComponent::TextSpriteComponent(Actor* owner, int drawOrder, VisualLayer layer)
: SpriteComponent(owner, drawOrder, layer)
, mText("")
, mColor(1.0f, 1.0f, 1.0f)
, mFont(nullptr)
{
}

TextSpriteComponent::~TextSpriteComponent()
{
    // mTexture は SpriteComponent / VisualComponent 側で shared_ptr 管理なので特に何もしなくてOK
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

void TextSpriteComponent::SetFont(std::shared_ptr<Font> font)
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

    auto* app = mOwnerActor->GetApp();
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

