#version 410 core

out vec4 FragColor;
in vec3 vWorldDir;

uniform float uTime;
uniform int uWeatherType;     // 0: Clear, 1: Cloudy, 2: Rain, 3: Storm, 4: Snow
uniform float uTimeOfDay;     // 0.0〜1.0（夜→昼→夜）
uniform vec3 uSunDir;         // 太陽ベクトル（ワールド空間）

/*
// --- ノイズ関数 ---
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
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
    float amp = 0.5;
    for (int i = 0; i < 4; ++i)
    {
        value += amp * noise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return value;
}
*/

// --- 置き換え：sinハッシュ禁止版 ---
// 高速・安定な2Dハッシュ（sin不使用）
float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.x, p.y, p.x) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Value noise（バイリニア補間）
float vnoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    // コーナーの乱数
    float a = hash12(i);
    float b = hash12(i + vec2(1.0, 0.0));
    float c = hash12(i + vec2(0.0, 1.0));
    float d = hash12(i + vec2(1.0, 1.0));

    // Hermite補間（= 元の u と同じ）
    vec2 u = f * f * (3.0 - 2.0 * f);

    // 2D補間
    return mix(mix(a, b, u.x),
               mix(c, d, u.x), u.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amp   = 0.5;
    // pが巨大化して精度落ちしないように、各オクターブでwrap
    for (int i = 0; i < 5; ++i) {
        value += amp * vnoise(p);
        p = (p * 2.0);
        p = mod(p, 1024.0); // ★ ここ重要：範囲を畳み込んでsin問題を根絶
        amp *= 0.5;
    }
    return value;
}

// --- 空の色（時間帯に応じて） ---
vec3 getSkyColor(float time)
{
    vec3 night = vec3(0.01, 0.02, 0.05);
    vec3 day   = vec3(0.7, 0.8, 1.0);
    vec3 dusk  = vec3(0.9, 0.4, 0.2);

    time = fract(time);
    if (time < 0.2)
    {
        return mix(night, dusk, smoothstep(0.0, 0.2, time));
    }
    else if (time < 0.4)
    {
        return mix(dusk, day, smoothstep(0.2, 0.4, time));
    }
    else if (time < 0.6)
    {
        return day;
    }
    else if (time < 0.8)
    {
        return mix(day, dusk, smoothstep(0.6, 0.8, time));
    }
    else
    {
        return mix(dusk, night, smoothstep(0.8, 1.0, time));
    }
}

// --- 雲の色（時間帯に応じて） ---
vec3 getCloudColor(float time)
{
    time = fract(time);
    vec3 dayColor   = vec3(1.0);
    vec3 duskColor  = vec3(0.5, 0.2, 0.2);
    vec3 nightColor = vec3(0.001, 0.001, 0.0015);

    if (time < 0.2)
        return mix(nightColor, duskColor, smoothstep(0.0, 0.2, time));
    else if (time < 0.4)
        return mix(duskColor, dayColor, smoothstep(0.2, 0.4, time));
    else if (time < 0.6)
        return dayColor;
    else if (time < 0.8)
        return mix(dayColor, duskColor, smoothstep(0.6, 0.8, time));
    else
        return mix(duskColor, nightColor, smoothstep(0.9, 1.0, time));
}

void main()
{
    float t = clamp(vWorldDir.y, 0.0, 1.0);
    float weatherFade = (uWeatherType == 0) ? 1.0 : 0.3;

    vec3 rawSky   = getSkyColor(uTimeOfDay);
    vec3 rawCloud = getCloudColor(uTimeOfDay);
    vec3 baseSky     = mix(vec3(0.4, 0.4, 0.5), rawSky, weatherFade);
    vec3 cloudColor  = mix(vec3(0.4, 0.4, 0.4), rawCloud, weatherFade);

    vec3 skyColor = mix(baseSky * 0.6, baseSky, t);

    float cloudAlpha = 0.0;

    // --- 雲ノイズ ---
    if (uWeatherType >= 0) // ← CLEARも対象にする
    {
        vec2 cloudUV = vWorldDir.xz * 10.0 + vec2(uTime * 0.05, 0.0);
        float density = fbm(cloudUV);

        // 天気ごとに個別設定
        if (uWeatherType == 0)
        {
            cloudAlpha = smoothstep(0.5, 0.75, density); // CLEAR：薄め
        }
        else if (uWeatherType == 1)
        {
            cloudAlpha = smoothstep(0.3, 0.6, density);  // CLOUDY：中程度
            cloudColor = vec3(0.6);
            skyColor = mix(skyColor, vec3(0.3), 0.5);
        }
        else if (uWeatherType == 2)
        {
            cloudAlpha = smoothstep(0.2, 0.5, density);  // RAIN：濃いめ
            skyColor *= 0.4;
            cloudColor = vec3(0.5);
        }
        else if (uWeatherType == 3)
        {
            cloudAlpha = smoothstep(0.2, 0.5, density);  // STORM：濃いめ
            skyColor = vec3(0.15);
            cloudColor = vec3(0.7);
        }
        else if (uWeatherType == 4)
        {
            cloudAlpha = smoothstep(0.3, 0.6, density);  // SNOW：ふわっと
            skyColor = vec3(0.30);
            cloudColor = vec3(0.9);
        }

        if (uWeatherType >= 2)
        {
            cloudAlpha = min(cloudAlpha + 0.4, 1.0);
        }
    }

    // --- 雷 ---
    if (uWeatherType == 3)
    {
        float flash = step(0.98, fract(sin(uTime * 12.0) * 43758.5453));
        skyColor += vec3(1.0) * flash * 0.8;
    }

    // --- 雪 ---
    if (uWeatherType == 4)
    {
        skyColor = mix(skyColor, cloudColor, 0.4);
        cloudAlpha = min(cloudAlpha + 0.2, 1.0);
    }

    // --- 雲合成 ---
    vec3 finalColor = mix(skyColor, cloudColor, cloudAlpha * 0.8);

    // --- 太陽ハイライト（雲に応じて透過） ---
    if (uWeatherType <= 1)
    {
        float sunAmount = clamp(dot(normalize(vWorldDir), -normalize(uSunDir)), 0.0, 1.0);
        vec3 sunGlow = vec3(1.2, 1.0, 0.8) * pow(sunAmount, 512.0);
        finalColor += sunGlow * (1.0 - cloudAlpha); // 雲が薄いほど強く見える

    }

    FragColor = vec4(finalColor, 1.0);
}
