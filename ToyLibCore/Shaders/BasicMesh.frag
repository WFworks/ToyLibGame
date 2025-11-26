#version 410

// 頂点シェーダーからのデータ
// テクスチャ座標
in vec2 fragTexCoord;
// 法線ベクトル
in vec3 fragNormal;
// 座標
in vec3 fragWorldPos;

// アウトプットする色
out vec4 outColor;

// テクスチャ
uniform sampler2D uTexture;

// ディレクショナルライト
struct DirectionalLight
{
    // 方向
    vec3 mDirection;
    // 拡散光色
    vec3 mDiffuseColor;
    // 反射光色
    vec3 mSpecColor;
};

// ライティングのUniforms
// カメラ位置
uniform vec3 uCameraPos;
// 反射光
uniform float uSpecPower;
// 環境光
uniform vec3 uAmbientLight;

// ディレクショナルライト
uniform DirectionalLight uDirLight;


void main()
{
    
    // 法線ベクトル
    vec3 N = normalize(fragNormal);
    // ライティング計算
    vec3 L = normalize(-uDirLight.mDirection);
    // 面からカメラのベクトル
    vec3 V = normalize(uCameraPos - fragWorldPos);
    // 反射角度
    vec3 R = normalize(reflect(-L, N));

    
    // 反射光を計算
    vec3 Phong = uAmbientLight;
    // テクスチャから色を計算
    vec4 col = texture(uTexture, fragTexCoord);

    //outColor = texture(uTexture, fragTexCoord) * vec4(Phong, col.a);
    outColor = col * vec4(Phong, col.a);
    

}
