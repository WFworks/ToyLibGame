#version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;
uniform float uRainAmount;
uniform float uSnowAmount;
uniform float uFogAmount;

const int SNOW_COUNT = 80;

// --- hash(float) と hash(vec2) 両方定義 ---
float hash(float x) {
    return fract(sin(x) * 43758.5453123);
}
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(27.619, 57.583))) * 43758.5453);
}

// --- ノイズ関数 ---
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}
float fbm(vec2 p) {
    float value = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        value += amp * noise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return value;
}

// --- 雨 ---
float rainPattern(vec2 uv) {
    uv *= vec2(300.0, 1.0);
    uv.y += uTime * 8.0;

    float id = floor(uv.x);
    float offset = hash(vec2(id, 0.0));
    float y = fract(uv.y + offset);
    float shape = smoothstep(0.0, 0.01, y) * (1.0 - y);
    return shape;
}

// --- 雪 ---
float snowPattern(vec2 uv) {
    float brightness = 0.0;
    for (int i = 0; i < SNOW_COUNT; ++i) {
        float fi = float(i);
        float x = hash(fi * 1.3) + sin(uTime * 0.2 + fi) * 0.01;
        float speed = 0.1 + hash(fi * 3.2) * 0.5;
        float y = fract(hash(fi * 2.1) - uTime * speed);
        vec2 snowPos = vec2(x, y);
        float dist = length(uv - snowPos);
        float size = 0.01 + hash(fi * 4.0) * 0.01;
        brightness += smoothstep(size, 0.0, dist);
    }
    return brightness;
}

// --- フォグ ---
float fogPattern(vec2 uv) {
    vec2 centeredUV = (gl_FragCoord.xy - 0.5 * uResolution) / uResolution.y;
    vec2 noiseUV = centeredUV * 1.5;
    float n = fbm(noiseUV + vec2(0.0, uTime * 0.02));
    return smoothstep(0.3, 1.0, n);
}

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    float alpha = 0.0;

    if (uRainAmount > 0.01) {
        float rain = rainPattern(uv);
        alpha += rain * uRainAmount * 0.25;
    }

    if (uSnowAmount > 0.01) {
        float snow = snowPattern(uv);
        alpha += snow * uSnowAmount * 1.2;
    }

    if (uFogAmount > 0.01) {
        float fog = fogPattern(uv);
        alpha += fog * uFogAmount * 0.9;
    }

    FragColor = vec4(vec3(1.0), clamp(alpha, 0.0, 1.0));
}

/*
#version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;
uniform float uRainAmount;
uniform float uFogAmount;
uniform float uSnowAmount;

const int SNOW_COUNT = 100;

// --- ハッシュ関数 ---
float hash(float x) {
    return fract(sin(x) * 43758.5453123);
}

// --- 雨パターン ---
float rainPattern(vec2 uv) {
    uv *= vec2(300.0, 1.0);
    uv.y += uTime * 8.0;

    float id = floor(uv.x);
    float offset = hash(id);
    float y = fract(uv.y + offset);

    float shape = smoothstep(0.0, 0.01, y) * (1.0 - y);
    return shape;
}

// --- 雪パターン（点を滑らかに） ---
float snowPattern(vec2 uv) {
    float brightness = 0.0;
    for (int i = 0; i < SNOW_COUNT; ++i) {
        float fi = float(i);

        // ランダムな位置と動き
        float x = hash(fi * 1.3) + sin(uTime * 0.2 + fi) * 0.01;
        float y = fract(hash(fi * 2.1) - uTime * (0.1 + hash(fi * 3.2) * 0.5));
        vec2 snowPos = vec2(x, y);

        float dist = length(uv - snowPos);
        float size = 0.008 + hash(fi * 4.0) * 0.01;
        brightness += smoothstep(size, 0.0, dist);
    }
    return brightness;
}

// --- FBMノイズ ---
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(27.619, 57.583))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
    float value = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        value += amp * noise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return value;
}

// --- フォグパターン（ノイズベース） ---
float fogPattern(vec2 uv) {
    uv = uv * 2.0 - 1.0;           // 中心基準にリマップ
    uv *= 1.5;                     // 模様のサイズ拡大
    float n = fbm(uv + vec2(0.0, uTime * 0.02));
    return smoothstep(0.4, 1.0, n);
}

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;

    float rain = uRainAmount > 0.01 ? rainPattern(uv) : 0.0;
    float snow = uSnowAmount > 0.01 ? snowPattern(uv) : 0.0;
    float fog  = uFogAmount > 0.01 ? fogPattern(uv) : 0.0;

    float alpha = 0.0;
    alpha += rain * uRainAmount * 0.25;
    alpha += snow * uSnowAmount * 1.2;
    alpha += fog * uFogAmount * 0.8;

    FragColor = vec4(vec3(1.0), clamp(alpha, 0.0, 1.0));
}
*/

/*
 #version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;

uniform float uRainAmount; // 0.0〜1.0
uniform float uFogAmount;
uniform float uSnowAmount;

#define SNOW_COUNT 64

// ランダム生成
float hash(float x)
{
    return fract(sin(x) * 43758.5453123);
}

// 雨（縦ストリーク）
float rainPattern(vec2 uv)
{
    uv *= vec2(300.0, 1.0);
    uv.y += uTime * 8.0;

    float id = floor(uv.x);
    float offset = hash(id);
    float y = fract(uv.y + offset);

    float shape = smoothstep(0.0, 0.01, y) * (1.0 - y);
    return shape;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / uResolution;

    // --- 雨 ---
    float rain = rainPattern(uv);

    // --- 雪 ---
    float snowBrightness = 0.0;
    if (uSnowAmount > 0.01)
    {
        for (int i = 0; i < SNOW_COUNT; ++i)
        {
            float fi = float(i);
            float x = hash(fi * 1.3) + sin(uTime * 0.2 + fi) * 0.01;
            float y = fract(hash(fi * 2.1) - uTime * (0.1 + hash(fi * 3.2) * 0.5));
            vec2 snowPos = vec2(x, y);
            float dist = length(uv - snowPos);
            float size = 0.008 + hash(fi * 4.0) * 0.008;
            snowBrightness += smoothstep(size, 0.0, dist);
        }
    }

    float snowAlpha = clamp(snowBrightness * uSnowAmount * 1.2, 0.0, 1.0);

    float snow = clamp(snowBrightness * uSnowAmount * 1.2, 0.0, 1.0);

    // --- フォグ ---
    float fog = uFogAmount;

    // --- 合成 ---
    float alpha = rain * uRainAmount * 0.25 +
                  snow +
                  fog * 0.5;

    FragColor = vec4(vec3(1.0), clamp(alpha, 0.0, 1.0));
}
*/

/*
#version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;
uniform float uRainAmount;  // 0.0〜1.0
uniform float uFogAmount;   // 0.0〜1.0
uniform float uSnowAmount;  // 0.0〜1.0

// --- ノイズ生成 ---
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(27.619, 57.583))) * 43758.5453);
}

// --- 雨パターン ---
float rainPattern(vec2 uv) {
    uv *= vec2(300.0, 1.0);            // 横に密度アップ
    uv.y += uTime * 8.0;               // 縦に高速落下

    float id = floor(uv.x);
    float offset = hash(vec2(id, 0.0));
    float y = fract(uv.y + offset);

    float shape = smoothstep(0.0, 0.01, y) * (1.0 - y); // 細長いストリーク
    return shape;
}
float snowPattern(vec2 uv) {
    uv *= vec2(60.0, 5.0);  // 横密度、縦も圧縮して範囲限定
    float id = floor(uv.x);

    float offset = hash(vec2(id, 0.0));
    float speed = 0.5 + hash(vec2(id, 1.0)) * 0.4;
    uv.y += uTime * speed + offset;

    float wobble = sin(uv.y * 10.0 + id) * 0.02;

    vec2 center = vec2(fract(uv.x + wobble) - 0.5, fract(uv.y) - 0.5);
    center.y *= 0.3;

    float dist = length(center);
    float flake = smoothstep(0.06, 0.0, dist); // 丸い雪粒

    return flake;
}
void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;

    float rain = rainPattern(uv);
    float snow = snowPattern(uv);
    float fog  = uFogAmount;

    float alpha = 0.0;
    alpha += rain * uRainAmount * 0.25;
    alpha += snow * uSnowAmount * 0.85; // ← 強めに
    alpha += fog * 0.6;

    vec3 overlayColor = mix(vec3(1.0), vec3(0.9), snow); // 雪影用

    FragColor = vec4(overlayColor, clamp(alpha, 0.0, 1.0));
}
*/
/*
#version 410 core

out vec4 FragColor;

uniform float uTime;
uniform vec2 uResolution;
uniform float uRainAmount;  // 0.0〜1.0
uniform float uFogAmount;   // 0.0〜1.0
uniform float uSnowAmount;  // 0.0〜1.0

// --- ノイズ生成 ---
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(27.619, 57.583))) * 43758.5453);
}

// --- 雨パターン ---
float rainPattern(vec2 uv) {
    uv *= vec2(300.0, 1.0);            // 横に密度アップ
    uv.y += uTime * 8.0;               // 縦に高速落下

    float id = floor(uv.x);
    float offset = hash(vec2(id, 0.0));
    float y = fract(uv.y + offset);

    float shape = smoothstep(0.0, 0.01, y) * (1.0 - y); // 細長いストリーク
    return shape;
}

// --- 雪パターン ---
float snowPattern(vec2 uv) {
    uv *= vec2(100.0, 1.0);
    uv.y += uTime * 1.5;

    float id = floor(uv.x);
    float offset = hash(vec2(id, 0.0));
    float y = fract(uv.y + offset);

    float wobble = sin(y * 20.0 + id) * 0.003; // 横揺れ
    float flake = smoothstep(0.9, 1.0, y) * (1.0 - y);

    return flake * (1.0 - abs(wobble * 300.0));
}

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;

    float rain = rainPattern(uv);
    float snow = snowPattern(uv);

    // --- 霧は画面全体に均一（今は単純な濃度） ---
    float fog = uFogAmount;

    // --- 合成アルファ ---
    float alpha = 0.0;
    alpha += rain * uRainAmount * 0.25;
    alpha += snow * uSnowAmount * 0.35;
    alpha += fog * 0.6;

    FragColor = vec4(vec3(1.0), clamp(alpha, 0.0, 1.0));
}
*/
