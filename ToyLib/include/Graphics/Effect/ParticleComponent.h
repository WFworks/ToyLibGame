// ParticleComponent.h
#pragma once
#include "Graphics/VisualComponent.h"
#include <vector>

namespace toy {

//======================================================================
// パーティクル描画コンポーネント
// - Sprite（テクスチャ）を複数生成してパーティクル風に描画する
// - 加算／通常ブレンドの切り替え
// - 雨・火花・煙などの簡易表現に利用
//======================================================================
class ParticleComponent : public VisualComponent
{
public:
    // -------------------------------------------
    // パーティクルの種類（挙動や拡散方向の違いに利用）
    // -------------------------------------------
    enum ParticleMode
    {
        P_SPARK,   // 火花系：高速・拡散
        P_WATER,   // 水滴系：重力 or 下方向
        P_SMOKE    // 煙系：上昇しながらフェード
    };
    
    // -------------------------------------------
    // 1パーティクルのデータ
    // -------------------------------------------
    struct Particle
    {
        Vector3 pos;      // 位置
        Vector3 dir;      // 移動方向（正規化推奨）
        float lifeTime;   // 残り寿命（秒）
        float size;       // サイズ（スケール）
        bool  isVisible;  // 描画するか
    };
    
    //==================================================================
    // コンストラクタ / デストラクタ
    //==================================================================
    ParticleComponent(class Actor* owner, int drawOrder = 20);
    ~ParticleComponent();
    
    //==================================================================
    // 更新処理（位置更新・寿命処理など）
    //==================================================================
    void Update(float deltaTime) override;
    
    //==================================================================
    // 描画（VisualComponent → フルビルボード描画）
    //==================================================================
    void Draw() override;
    
    //==================================================================
    // テクスチャ設定（VisualComponent のオーバーライド）
    //==================================================================
    void SetTexture(std::shared_ptr<class Texture> tex) override;
    
    //==================================================================
    // パーティクル生成
    // pos        : 発生位置
    // num        : 発生数
    // life       : コンポーネント自体の生存時間（0で無限）
    // partLife   : パーティクル1個の寿命
    // size       : パーティクルの基本サイズ
    // mode       : 種類（Spark, Water, Smoke）
    //==================================================================
    void CreateParticles(
        Vector3 pos,
        unsigned int num,
        float life,
        float partLife,
        float size,
        ParticleMode mode
    );
    
    //==================================================================
    // ブレンド設定
    // true  → 加算合成（発光系）
    // false → 透過ブレンド（煙・水）
    //==================================================================
    void SetAddBlend(bool b) { mIsBlendAdd = b; }
    
    // パーティクル速度係数
    void SetSpeed(float speed) { mPartSpeed = speed; }
    
    // 描画順序
    int GetDrawOrder() const { return mDrawOrder; }
    
private:
    //==================================================================
    // 生成されたパーティクルの初期化（内部用）
    //==================================================================
    void GenerateParts();
    
    //==================================================================
    // メンバ変数
    //==================================================================
    std::shared_ptr<class Texture> mTexture;   // パーティクル用テクスチャ
    Vector3 mPosition;                         // 発生位置
    std::vector<Particle> mParts;              // 生成済みパーティクル一覧
    
    int mDrawOrder;                            // 描画順
    unsigned int mNumParts;                    // 初期生成数
    float mLifeTime;                           // コンポーネント全体の寿命
    float mTotalLife;                          // 経過時間
    float mPartLifecycle;                      // パーティクル1個の寿命
    float mPartSize;                           // パーティクルサイズ
    float mPartSpeed;                          // 速度係数
    
    ParticleMode mParticleMode;                // モード（挙動）
    bool mIsBlendAdd;                          // 加算合成かどうか
};

} // namespace toy
