#version 410

// 頂点シェーダーから受け取るUV座標
in vec2 fragTexCoord;

// 座標
in vec3 fragWorldPos;

// 出力する色
out vec4 outColor;

// Uniform テクスチャ
uniform sampler2D uTexture;


// カメラ位置
uniform vec3 uCameraPos;

struct FogInfo {
    float maxDist;
    float minDist;
    vec3 color;
};
// フォグ情報
uniform FogInfo uFoginfo;


void main()
{

    // フォグの値を計算
    float dist = length(uCameraPos - fragWorldPos);
    float fogFactor = (uFoginfo.maxDist - dist) / (uFoginfo.maxDist - uFoginfo.minDist);
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);
    

    // テクスチャから色を計算
    vec4 col = texture(uTexture, fragTexCoord);
    
    
  
    // フォグを適用
    vec3 c = mix(uFoginfo.color, col.rgb, fogFactor);
    outColor = vec4(c, col.a);

   
}
