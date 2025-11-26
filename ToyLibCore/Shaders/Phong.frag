#version 410

// === 入力 ===
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragWorldPos;
in vec4 fragPosLightSpace;  // ← 追加！

// 出力
out vec4 outColor;

// === Uniforms ===
uniform sampler2D uTexture;
uniform vec3 uUniformColor;
uniform bool uOverrideColor;
uniform vec3 uCameraPos;
uniform float uSpecPower;
uniform vec3 uAmbientLight;
uniform float uShadowBias;
uniform bool uUseToon;

// Directional Light
struct DirectionalLight {
    vec3 mDirection;
    vec3 mDiffuseColor;
    vec3 mSpecColor;
};
uniform DirectionalLight uDirLight;

// Fog
struct FogInfo {
    float maxDist;
    float minDist;
    vec3 color;
};
uniform FogInfo uFoginfo;

// Shadow Mapping
uniform sampler2DShadow uShadowMap;

// === 定数 ===
const float toonDiffuseThreshold = 0.5;
const float toonSpecThreshold = 0.95;

// === 関数：ライティング計算 ===
vec3 ComputeLighting(vec3 N, vec3 V, vec3 L)
{
    vec3 result = uAmbientLight;
    float NdotL = dot(N, L);

    if (NdotL > 0.0)
    {
        if (uUseToon)
        {
            float diffIntensity = step(toonDiffuseThreshold, NdotL);
            float specIntensity = pow(max(dot(reflect(-L, N), V), 0.0), uSpecPower);
            specIntensity = step(toonSpecThreshold, specIntensity);

            result += uDirLight.mDiffuseColor * diffIntensity;
            result += uDirLight.mSpecColor * specIntensity;
        }
        else
        {
            vec3 diffuse = uDirLight.mDiffuseColor * NdotL;
            vec3 specular = uDirLight.mSpecColor *
                pow(max(dot(reflect(-L, N), V), 0.0), uSpecPower);
            result += diffuse + specular;
        }
    }

    return result;
}

// === 関数：シャドウ判定 ===
float ComputeShadow()
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0; // ライトが当たってる扱い
    }

    float shadow = textureProj(uShadowMap, vec4(projCoords.xy, projCoords.z - uShadowBias, 1.0));
    return mix(0.5, 1.0, shadow); // ソフトシャドウ効果
}

// === メイン ===
void main()
{
    // フォグ計算
    float dist = length(uCameraPos - fragWorldPos);
    float fogFactor = clamp((uFoginfo.maxDist - dist) / (uFoginfo.maxDist - uFoginfo.minDist), 0.0, 1.0);

    if (uOverrideColor)
    {
        outColor = vec4(mix(uFoginfo.color, uUniformColor, fogFactor), 1.0);
        return;
    }

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(uCameraPos - fragWorldPos);
    vec3 L = normalize(-uDirLight.mDirection);
    
    vec3 lighting = ComputeLighting(N, V, L);
    float shadowFactor = ComputeShadow();

    vec4 texColor = texture(uTexture, fragTexCoord);
    texColor.rgb *= lighting * shadowFactor;

    vec3 finalColor = mix(uFoginfo.color, texColor.rgb, fogFactor);
    outColor = vec4(finalColor, texColor.a);
}
/*
#version 410

// === 入力 ===
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragWorldPos;

// 出力
out vec4 outColor;

// === Uniforms ===
uniform sampler2D uTexture;
uniform vec3 uUniformColor;
uniform bool uOverrideColor;
uniform vec3 uCameraPos;
uniform float uSpecPower;
uniform vec3 uAmbientLight;
uniform float uShadowBias;       // ← シャドウバイアスを外部から制御
uniform bool uUseToon;           // ← トゥーン有効/無効切り替え

// Directional Light
struct DirectionalLight {
    vec3 mDirection;
    vec3 mDiffuseColor;
    vec3 mSpecColor;
};
uniform DirectionalLight uDirLight;

// Fog
struct FogInfo {
    float maxDist;
    float minDist;
    vec3 color;
};
uniform FogInfo uFoginfo;

// Shadow Mapping
uniform mat4 uLightSpaceMatrix;
uniform sampler2DShadow uShadowMap;

// === 定数 ===
const float toonDiffuseThreshold = 0.5;
const float toonSpecThreshold = 0.95;

// === 関数：ライティング計算 ===
vec3 ComputeLighting(vec3 N, vec3 V, vec3 L)
{
    vec3 result = uAmbientLight;
    float NdotL = dot(N, L);

    if (NdotL > 0.0)
    {
        if (uUseToon)
        {
            float diffIntensity = step(toonDiffuseThreshold, NdotL);
            float specIntensity = pow(max(dot(reflect(-L, N), V), 0.0), uSpecPower);
            specIntensity = step(toonSpecThreshold, specIntensity);

            result += uDirLight.mDiffuseColor * diffIntensity;
            result += uDirLight.mSpecColor * specIntensity;
        }
        else
        {
            vec3 diffuse = uDirLight.mDiffuseColor * NdotL;
            vec3 specular = uDirLight.mSpecColor *
                pow(max(dot(reflect(-L, N), V), 0.0), uSpecPower);
            result += diffuse + specular;
        }
    }

    return result;
}

// === 関数：シャドウ判定 ===
float ComputeShadow(vec3 worldPos)
{
    vec4 lightSpacePos = vec4(worldPos, 1.0) * uLightSpaceMatrix;
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 1.0; // ライトが当たってる扱い
    }

    float shadow = textureProj(uShadowMap, vec4(projCoords.xy, projCoords.z - uShadowBias, 1.0));
    return mix(0.5, 1.0, shadow); // ソフトシャドウ効果
}

// === メイン ===
void main()
{
    // === フォグ計算 ===
    float dist = length(uCameraPos - fragWorldPos);
    float fogFactor = clamp((uFoginfo.maxDist - dist) / (uFoginfo.maxDist - uFoginfo.minDist), 0.0, 1.0);

    // === 強制カラーならそれを表示 ===
    if (uOverrideColor)
    {
        outColor = vec4(mix(uFoginfo.color, uUniformColor, fogFactor), 1.0);
        return;
    }

    // === ライティングとシャドウ ===
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(uCameraPos - fragWorldPos);
    vec3 L = normalize(-uDirLight.mDirection);
    
    vec3 lighting = ComputeLighting(N, V, L);
    float shadowFactor = ComputeShadow(fragWorldPos);

    // === テクスチャ色 × ライティング × シャドウ ===
    vec4 texColor = texture(uTexture, fragTexCoord);
    texColor.rgb *= lighting * shadowFactor;

    // === フォグ適用 ===
    vec3 finalColor = mix(uFoginfo.color, texColor.rgb, fogFactor);
    outColor = vec4(finalColor, texColor.a);
}

*/
