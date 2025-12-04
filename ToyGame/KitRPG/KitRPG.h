#pragma once

#include "Kit/Game.h"

class KitRPG : public toy::kit::Game
{
public:
    KitRPG();
    virtual ~KitRPG() = default;

protected:
    void Setup() override;
    void Tick(float deltaTime) override;
};
