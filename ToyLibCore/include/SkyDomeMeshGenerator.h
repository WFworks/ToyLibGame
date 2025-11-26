#pragma once

#include "Renderer.h"

namespace SkyDomeMeshGenerator {

// スカイドーム用の半球メッシュを生成して返す（Rendererに登録）
std::unique_ptr<class VertexArray> CreateSkyDomeVAO(int slices = 32, int stacks = 16, float radius = 1.0f);

}
