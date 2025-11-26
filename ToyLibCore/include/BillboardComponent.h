#pragma once

#include "VisualComponent.h"
#include <memory>

class BillboardComponent : public VisualComponent
{
public:
    BillboardComponent(class Actor* a, int drawOrder);
    ~BillboardComponent();

    void Draw() override;
    
private:
    float mScale;
};
