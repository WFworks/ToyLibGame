#pragma once

#include "Kit/Scene.h"
#include "Kit/Stage.h"
#include "Kit/Character.h"

class FieldScene : public toy::kit::Scene
{
public:
    FieldScene();
    virtual ~FieldScene() = default;

    void OnEnter() override;
    void OnExit() override;
    void Update(float deltaTime) override;

private:
    toy::kit::Stage     mStage;
    toy::kit::Character mPlayer;
};
