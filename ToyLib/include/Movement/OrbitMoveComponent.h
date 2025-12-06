#pragma once

#include "Movement/MoveComponent.h"
#include "Utils/MathUtil.h"

namespace toy {

//------------------------------------------------------------------------------
// OrbitMoveComponent
//------------------------------------------------------------------------------
// ・指定した中心 Actor の周囲を「公転」する MoveComponent。
// ・半径・角速度を指定し、XZ 平面上で円運動させる用途に最適。
// ・敵の周回、ドローンの巡回、魔法エフェクトの回転などに使える。
//------------------------------------------------------------------------------
class OrbitMoveComponent : public MoveComponent
{
public:
    OrbitMoveComponent(class Actor* owner, int updateOrder = 10);
    
    // 公転処理（角度更新→位置更新）
    void Update(float deltaTime) override;
    
    //--- パラメータ設定 -------------------------------------------------------
    void SetCenterActor(class Actor* center) { mCenterActor = center; }
    void SetOrbitRadius(float radius)        { mOrbitRadius = radius; }
    void SetOrbitSpeed(float speed)          { mOrbitSpeed  = speed; }
    
private:
    //--- 公転パラメータ -------------------------------------------------------
    class Actor* mCenterActor;  // 公転の中心となるアクター
    float mOrbitRadius;         // 公転の半径
    float mOrbitSpeed;          // 公転速度（度/秒 or ラジアン/秒）
    float mCurrentAngle;        // 現在の角度（内部累積）
};

} // namespace toy
