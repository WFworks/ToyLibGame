#pragma once

#include "Graphics/Sprite/SpriteComponent.h"
#include "Utils/StringUtil.h"
#include <string>
#include <memory>




// テキストを UI スプライトとして表示するコンポーネント
class TextSpriteComponent : public SpriteComponent
{
public:
    // layer は基本 VisualLayer::UI 固定で使う想定
    TextSpriteComponent(class Actor* owner, int drawOrder = 100, VisualLayer layer = VisualLayer::UI);
    virtual ~TextSpriteComponent();

    // テキスト内容を変更
    void SetText(const std::string& text);
    
    // フォーマット付き
    template<typename... Args>
    void SetFormat(const std::string& fmt, Args&&... args)
    {
        SetText(StringUtil::Format(fmt, std::forward<Args>(args)...));
    }


    // 色 (0.0〜1.0)
    void SetColor(const Vector3& color);

    // 使用するフォント（AssetManager から取得した shared_ptr をそのまま渡す）
    void SetFont(std::shared_ptr<TextFont> font);

    // 今の設定を元にテクスチャだけ作り直したい場合に呼べる
    void Refresh();

    const std::string& GetText() const { return mText; }
    const Vector3& GetColor() const { return mColor; }
    std::shared_ptr<class TextFont> GetFont() const { return mFont; }

private:
    void UpdateTexture();

    std::string mText;
    Vector3 mColor;
    std::shared_ptr<class TextFont> mFont;   // 所有権は AssetManager と共有
};
