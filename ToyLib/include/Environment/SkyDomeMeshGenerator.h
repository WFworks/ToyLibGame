#pragma once

#include "Engine/Render/Renderer.h"

namespace toy {
namespace SkyDomeMeshGenerator {

/**
 * @brief スカイドーム用の半球メッシュ（VertexArray）を生成する。
 *
 * - 天球（スカイドーム）の描画で使用する半球ジオメトリを作成する。
 * - slices：経度方向の分割数
 * - stacks：緯度方向の分割数
 * - radius：半球の半径
 *
 * ※ 実際の空の描画（色や天候）は SkyDomeComponent / WeatherDomeComponent が担当するため、
 *    本関数はジオメトリ生成に特化する。
 *
 * @return 生成済み VertexArray（index + vertex buffer を含む）
 */
std::unique_ptr<class VertexArray> CreateSkyDomeVAO(
    int slices = 32,
    int stacks = 16,
    float radius = 1.0f
);

} // namespace SkyDomeMeshGenerator
} // namespace toy
