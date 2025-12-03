#pragma once

#include "Graphics/VisualComponent.h"
#include <memory>

namespace toy {

class BillboardComponent : public VisualComponent
{
public:
    BillboardComponent(class Actor* a, int drawOrder);
    ~BillboardComponent();
    
    void Draw() override;
    
private:
    float mScale;
};

} // namespace toy
