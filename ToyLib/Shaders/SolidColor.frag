#version 410

// 頂点シェーダーからのデータ
// 法線ベクトル
in vec3 fragNormal;
// 座標
in vec3 fragWorldPos;
// アウトプットする色
out vec4 outColor;

// ライティングのUniforms
// 環境光
uniform vec3 uAmbientLight;
// 色
uniform vec3 uSolColor;


void main()
{
    // 反射光を計算
    vec3 Phong = uAmbientLight;
    //
    outColor = vec4(uSolColor, 1.0f) * vec4(Phong, 1.0f);
    

}
