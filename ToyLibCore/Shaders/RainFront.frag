#version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;
uniform float uRainAmount;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(27.619, 57.583))) * 43758.5453);
}

float rainPattern(vec2 uv) {
    uv *= vec2(300.0, 1.0);            // ← 横密度UP
    uv.y += uTime * 8.0;              // ← 落下速度速め

    float id = floor(uv.x);
    float offset = hash(vec2(id, 0.0));    // ランダム開始位置

    float y = fract(uv.y + offset);
    
    // 薄くて細長いストリーク
    float brightness = smoothstep(0.0, 0.01, y) * (1.0 - y); // 徐々に消える尻尾感

    return brightness;
}
void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;
    float rain = rainPattern(uv);

    float alpha = rain * uRainAmount * 0.25; // 弱めにする
    FragColor = vec4(vec3(1.0), alpha);
}
