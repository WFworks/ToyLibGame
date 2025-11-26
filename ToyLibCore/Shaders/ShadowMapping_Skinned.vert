#version 410

// ボーン行列
uniform mat4 uMatrixPalette[96];

// ワールド変換（通常必要ない場合もあるが保険で入れる）
uniform mat4 uWorldTransform;

// ライト空間行列（これがポイント！）
uniform mat4 uLightSpaceMatrix;

// 頂点入力
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uvec4 inSkinBones;
layout(location = 4) in vec4 inSkinWeights;

void main()
{
    vec4 pos = vec4(inPosition, 1.0);

    // スキニング行列生成
    mat4 skinnedMat =
        uMatrixPalette[inSkinBones[0]] * inSkinWeights[0] +
        uMatrixPalette[inSkinBones[1]] * inSkinWeights[1] +
        uMatrixPalette[inSkinBones[2]] * inSkinWeights[2] +
        uMatrixPalette[inSkinBones[3]] * inSkinWeights[3];

    // スキニング → ワールド → ライト空間
    //vec4 skinnedPos = uWorldTransform * (skinnedMat * pos);
    vec4 skinnedPos = pos * skinnedMat;
    skinnedPos = skinnedPos * uWorldTransform;
    //gl_Position = uLightSpaceMatrix * uWorldTransform * vec4(inPosition, 1.0);
    gl_Position = skinnedPos * uLightSpaceMatrix;
}
