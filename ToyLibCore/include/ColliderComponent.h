#pragma once

#include "Component.h"
#include "Polygon.h"

#include <vector>
#include <memory>

#define ENABLE_BITMASK_OPERATORS(x)                                \
inline x operator|(x a, x b) { return static_cast<x>(static_cast<int>(a) | static_cast<int>(b)); } \
inline x operator&(x a, x b) { return static_cast<x>(static_cast<int>(a) & static_cast<int>(b)); } \
inline x& operator|=(x& a, x b) { a = a | b; return a; }            \
inline x& operator&=(x& a, x b) { a = a & b; return a; }            \
inline x operator~(x a) { return static_cast<x>(~static_cast<int>(a)); }

enum ColliderType : uint32_t
{
    C_NONE    = 0,
    C_PLAYER  = 1 << 0,
    C_ENEMY   = 1 << 1,
    C_BULLET  = 1 << 2,
    C_LASER   = 1 << 3,
    C_WALL    = 1 << 4,
    C_GROUND  = 1 << 5,
    C_FOOT    = 1 << 6,
};
ENABLE_BITMASK_OPERATORS(ColliderType)


class ColliderComponent : public Component
{
public:
    ColliderComponent(class Actor* a);
    virtual ~ColliderComponent();

    // 自分のタイプ
    void SetFlags(uint32_t flags) { mFlags = flags; }
    void AddFlag(uint32_t flag) { mFlags |= flag; }
    void RemoveFlag(uint32_t flag) { mFlags &= ~flag; }
    bool HasFlag(uint32_t flag) const { return (mFlags & flag) != 0; }
    bool HasAnyFlag(uint32_t flags) const { return (mFlags & flags) != 0; }
    uint32_t GetFlags() const { return mFlags; }
    
    // 衝突した
    void Collided(ColliderComponent* c);
    
    const std::vector<ColliderComponent*>& GetTargetColliders() const { return mTargetColliders; }
    void ClearCollidBuffer() { mTargetColliders.clear(); }
    
    void Update(float deltaTime) override;
    
    //class Actor* GetActor() const { return mOwner; }
    class BoundingVolumeComponent* GetBoundingVolume() const { return mBoundingVolume; }
    
    // 衝突しているかどうか
    bool GetCollided() const { return mIsCollided; }
    void SetCollided(bool b) { mIsCollided = b; }
    
    // 表示されているかどうか
    bool GetDisp() const { return mIsDisp; }
    void SetDisp(bool b) { mIsDisp = b; }

    // レイを取得 基本は空
    virtual Ray GetRay() const { return Ray(); }
    
private:
    
    bool mIsCollided;
    bool mIsDisp;
    
    class BoundingVolumeComponent* mBoundingVolume;
    
    uint32_t mFlags;
    std::vector<ColliderComponent*> mTargetColliders;
    
};
