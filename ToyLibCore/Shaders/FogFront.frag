#version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;
uniform float uFogAmount;

// --- ハッシュベース簡易ノイズ関数 ---
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 4; ++i) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    // 正規化UV
    vec2 uv = gl_FragCoord.xy / uResolution;
    
    // アスペクト比補正付き中心ベクトル
    vec2 centeredUV = uv - 0.5;
    centeredUV.x *= uResolution.x / uResolution.y;
    
    // 距離からベースの霧強度
    float dist = length(centeredUV);
    float baseFog = smoothstep(0.3, 0.8, dist);

    // 動くノイズ（ゆらぎ）
    float n = fbm(centeredUV * 3.5 + vec2(uTime * 0.05, 0.0));
    
    // ゆらぎを合成
    float fog = baseFog * mix(0.7, 1.0, n);

    // 最終出力（白い霧、アルファ付き）
    FragColor = vec4(vec3(1.0), fog * uFogAmount);
}

/*
#version 410 core

out vec4 FragColor;
in vec2 vTexCoord;

uniform float uTime;
uniform vec2 uResolution;
uniform float uFogAmount;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0); // ← 強制的に正規化
    float dist = length(uv - 0.5);
    float fog = smoothstep(0.4, 0.8, dist);

    FragColor = vec4(1.0, 1.0, 1.0, fog);
}
*/
/*
// --- fbmノイズ（柔らかく滑らか）
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f*f*(3.0 - 2.0*f);
    return mix(a, b, u.x) + (c - a)*u.y*(1.0 - u.x) + (d - b)*u.x*u.y;
}
float fbm(vec2 p) {
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 5; ++i) {
        v += amp * noise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return v;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;

    // 擬似的に「奥に向かう」感（中央が薄く、端が濃く）
    float depthFactor = smoothstep(0.4, 1.0, length(uv - 0.5));

    // 動くノイズ
    float fogNoise = fbm(uv * 3.0 + vec2(uTime * 0.05, 0.0));

    // 霧の最終強度
    float fogAlpha = fogNoise * depthFactor * uFogAmount * 0.4;

    FragColor = vec4(vec3(1.0), fogAlpha);
}
*/
