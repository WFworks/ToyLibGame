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



void main()
{
    // テクスチャから色を計算
    outColor= texture(uTexture, fragTexCoord);

   
}
