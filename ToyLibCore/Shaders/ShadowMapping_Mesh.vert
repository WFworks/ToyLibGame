#version 410

// === Uniforms ===
uniform mat4 uWorldTransform;
uniform mat4 uLightSpaceMatrix;

// === Attributes ===
layout(location = 0) in vec3 inPosition;

void main()
{
    vec4 pos = vec4(inPosition, 1.0);

    // ライト空間へ変換（projection * view）
    gl_Position =  pos * uWorldTransform * uLightSpaceMatrix;
    // 深度値だけ出力されるため、fragは不要
}
