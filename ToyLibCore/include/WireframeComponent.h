#pragma once

#include "VisualComponent.h"
#include <memory>


// ワイヤーフレームの表示用
class WireframeComponent : public VisualComponent
{
public:
    WireframeComponent(class Actor* owner, int drawOrder, VisualLayer layer = VisualLayer::Object3D);
    
    void Draw() override;
    
    void SetVertexArray(std::shared_ptr<class VertexArray> vertex) { mVertexArray = vertex; }
    
    void SetColor(const Vector3& color) { mColor = color; }
private:
    Vector3 mColor;
    
};
