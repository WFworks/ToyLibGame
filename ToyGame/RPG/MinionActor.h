#pragma once

#include "ToyLib.h"


class MinionActor : public Actor
{
public:
    MinionActor(class Application* a);
    virtual ~MinionActor();
    void UpdateActor(float deltaTime) override;
private:
    float mCounter;

};

