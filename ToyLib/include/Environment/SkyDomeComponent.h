#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Render/Shader.h"
#include <memory>

namespace toy {

//======================================
// SkyDomeComponent
//  - 「空の半球メッシュ」を描画するためのベースコンポーネント
//  - 実際の見た目（時間帯・天候など）は派生クラス側で制御する想定
//
//  役割イメージ：
//   * スカイドーム用の VertexArray / Shader / LightingManager を保持する土台
//   * Renderer 側から「SkyDome として描画して」と呼ばれる窓口
//   * WeatherDomeComponent などが継承して、Draw/Update を上書きする
//======================================
class SkyDomeComponent : public Component
{
public:
    // コンストラクタ
    //  - 派生クラス側でメッシュ生成・Rendererへの登録・シェーダ取得などを行う前提
    SkyDomeComponent(class Actor* a);
    
    // スカイドーム描画
    //  - ベースクラスでは何もしない
    //  - WeatherDomeComponent などの派生クラスでオーバーライドして描画処理を書く
    virtual void Draw();
    
    // 更新処理
    //  - ベースクラスでは特別な処理は持たない
    //  - 派生クラス側で時間や天候に応じた更新処理を行う想定
    void Update(float deltaTime) override;
    
    // ライティング管理クラスの設定
    //  - 太陽方向・アンビエント色・フォグ色などを共有するために使用
    void SetLightingManager(std::shared_ptr<class LightingManager> manager)
    {
        mLightingManager = manager;
    }
    
protected:
    // スカイドーム用メッシュ（半球の VertexArray）
    std::unique_ptr<class VertexArray> mSkyVAO;
    
    // ライティング制御（太陽・月・フォグなど）
    std::shared_ptr<class LightingManager> mLightingManager;
    
    // スカイドーム描画用シェーダ
    //  - WeatherDomeComponent では "SkyDome" シェーダを想定
    std::shared_ptr<class Shader> mShader;
};

} // namespace toy
