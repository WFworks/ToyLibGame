#version 410 core

out vec4 FragColor;
in vec3 vWorldDir;

uniform float uTime;
uniform int  uWeatherType;     // 0: Clear, 1: Cloudy, 2: Rain, 3: Storm, 4: Snow
uniform float uTimeOfDay;      // 0.0〜1.0（夜→昼→夜）
uniform vec3 uSunDir;          // 太陽ベクトル（ワールド空間）

uniform vec3 uRawSkyColor;
uniform vec3 uRawCloudColor;

//======================
// ハッシュ / ノイズ
//======================
float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.x, p.y, p.x) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float vnoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash12(i);
    float b = hash12(i + vec2(1.0, 0.0));
    float c = hash12(i + vec2(0.0, 1.0));
    float d = hash12(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(a, b, u.x),
               mix(c, d, u.x), u.y);
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amp   = 0.5;
    for (int i = 0; i < 5; i++)
    {
        value += amp * vnoise(p);
        p = mod(p * 2.0, 1024.0);
        amp *= 0.5;
    }
    return value;
}

// 3D ハッシュ
float hash13(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

// 3D value noise
float vnoise3(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);

    float n000 = hash13(i + vec3(0.0, 0.0, 0.0));
    float n100 = hash13(i + vec3(1.0, 0.0, 0.0));
    float n010 = hash13(i + vec3(0.0, 1.0, 0.0));
    float n110 = hash13(i + vec3(1.0, 1.0, 0.0));
    float n001 = hash13(i + vec3(0.0, 0.0, 1.0));
    float n101 = hash13(i + vec3(1.0, 0.0, 1.0));
    float n011 = hash13(i + vec3(0.0, 1.0, 1.0));
    float n111 = hash13(i + vec3(1.0, 1.0, 1.0));

    vec3 u = f * f * (3.0 - 2.0 * f);

    float nx00 = mix(n000, n100, u.x);
    float nx10 = mix(n010, n110, u.x);
    float nx01 = mix(n001, n101, u.x);
    float nx11 = mix(n011, n111, u.x);

    float nxy0 = mix(nx00, nx10, u.y);
    float nxy1 = mix(nx01, nx11, u.y);

    return mix(nxy0, nxy1, u.z);
}

float fbm3(vec3 p)
{
    float value = 0.0;
    float amp   = 0.5;

    for (int i = 0; i < 5; i++)
    {
        value += amp * vnoise3(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return value;
}


//======================
// メイン
//======================
void main()
{
    vec3 dir = normalize(vWorldDir);

    float t = clamp(vWorldDir.y, 0.0, 1.0);
    float weatherFade = (uWeatherType == 0) ? 1.0 : 0.3;

    // 昼夜の強さ（C++側と同じロジック）
    float dayStrength =
        smoothstep(0.15, 0.25, uTimeOfDay) *
        (1.0 - smoothstep(0.75, 0.85, uTimeOfDay));
    float nightStrength = 1.0 - dayStrength;

    vec3 rawSky   = uRawSkyColor;
    vec3 rawCloud = uRawCloudColor;
    vec3 baseSky     = mix(vec3(0.4, 0.4, 0.5), rawSky, weatherFade);
    vec3 cloudColor  = mix(vec3(0.4, 0.4, 0.4), rawCloud, weatherFade);

    vec3 skyColor = mix(baseSky * 0.6, baseSky, t);

    float cloudAlpha = 0.0;

    // --- 雲ノイズ ---
    {
        vec3 p = dir * 7.0; // 8 → 7 で少しだけスケール大きめ（雲が気持ち大きく）
        p.xz += vec2(uTime * 0.03, uTime * 0.01);

        float density = fbm3(p);

        if (uWeatherType == 0)         // CLEAR
        {
            // 0.5,0.75 → 0.38,0.7 くらいに下げて、ちょっと雲多めの快晴に
            cloudAlpha = smoothstep(0.38, 0.70, density);
        }
        else if (uWeatherType == 1)    // CLOUDY
        {
            // 0.3,0.6 → 0.20,0.55 にして、かなり「曇天」寄りに
            cloudAlpha = smoothstep(0.20, 0.55, density);
            cloudColor = vec3(0.6);
            skyColor   = mix(skyColor, vec3(0.3), 0.6); // 0.5 → 0.6 で空もさらに鈍く
        }
        else if (uWeatherType == 2)    // RAIN
        {
            // 0.2,0.5 → 0.15,0.45 でほぼ一面雲
            cloudAlpha = smoothstep(0.15, 0.45, density);
            skyColor *= 0.35;          // 0.4 → 0.35 で暗めに
            cloudColor = vec3(0.5);
        }
        else if (uWeatherType == 3)    // STORM
        {
            cloudAlpha = smoothstep(0.15, 0.45, density);
            skyColor   = vec3(0.12);
            cloudColor = vec3(0.7);
        }
        else if (uWeatherType == 4)    // SNOW
        {
            // 雪も「空の抜け」は少なめに
            cloudAlpha = smoothstep(0.22, 0.50, density);
            skyColor   = vec3(0.30);
            cloudColor = vec3(0.9);
        }

        if (uWeatherType >= 2)
        {
            // ベース +0.4 は維持しつつ、あとで mix 係数も上げる
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


    // ======================
    // ★ 夜空の星
    // ======================
    if (nightStrength > 0.01 && uWeatherType == 0)
    {
        // 上方向ほど星が見えやすい（低緯度もそこそこ）
        float up = clamp(dir.y, 0.0, 1.0);
        float upFade = smoothstep(0.0, 0.4, up);

        // 星の散布パターン（密度＆粗さ）
        vec2 starUV = dir.xz * 40.0;

        float seed = hash12(starUV);

        // 星のマスク（少し幅を持たせて粒を大きく）
        float starMask = smoothstep(0.9985, 1.0, seed);

        float cloudBlock = 1.0 - cloudAlpha;
        vec3  starColor  = vec3(1.0, 0.97, 0.92);

        float starIntensity = starMask * nightStrength * upFade * cloudBlock;

        finalColor += starColor * starIntensity;
    }


    // ======================
    // ★ 月 (Moon)
    // ======================
    if (nightStrength > 0.01 && uWeatherType == 0)
    {
        // ひとまず固定方向（そのうち uniform にしてもOK）
        vec3 moonDir = normalize(vec3(-0.3, 0.9, 0.2));

        float m = clamp(dot(dir, moonDir), 0.0, 1.0);

        // 本体ディスク
        float moonDisk = smoothstep(0.985, 1.0, m);

        // グローとハロ（控えめ）
        float moonGlow = pow(m, 4096.0);
        float halo     = pow(m, 10.0);

        vec3 moonColor = vec3(1.2, 1.15, 1.0);
        float cloudBlock = 1.0 - cloudAlpha;

        float moonIntensity =
            (moonDisk * 1.0 +
             moonGlow * 0.6 +
             halo     * 0.4) *
            nightStrength * cloudBlock;

        finalColor += moonColor * moonIntensity;
    }


    // ======================
    // ★ 天の川 (Milky Way) ※継ぎ目対策済み
    // ======================
    if (nightStrength > 0.01 && uWeatherType == 0)
    {
        // 帯の方向（頭上を斜めに横切るイメージ）
        vec3 bandDir = normalize(vec3(0.5, -0.2, 0.0));

        // dir と bandDir の角度差
        float milky = abs(dot(dir, bandDir));

        // 帯の太さ（数値をいじると幅が変わる）
        float band = smoothstep(0.5, 0.2, milky);

        // ★ 3D ノイズで継ぎ目なし
        float noise = fbm3(dir * 4.0 + vec3(0.0, uTime * 0.02, 0.0));

        float milkyMask = band * noise * nightStrength * (1.0 - cloudAlpha);

        vec3 milkyColor = vec3(0.75, 0.8, 1.0);
        milkyColor = mix(milkyColor, vec3(1.0, 0.8, 0.9), noise * 0.3);

        finalColor += milkyColor * milkyMask * 0.7;
    }


    // --- 太陽ハイライト（雲に応じて透過） ---
    if (uWeatherType <= 1)
    {
        float hour = uTimeOfDay * 24.0;

        const float sunriseHour  = 5.0;   // 日の出
        const float sunsetHour   = 18.5;  // 日の入り（ちょい長めに残すならここを調整）
        const float dawnSpanHour = 1.0;
        const float duskSpanHour = 1.5;

        float sunVisibility = 0.0;

        // 日の出前後フェードイン
        if (hour >= sunriseHour - dawnSpanHour && hour < sunriseHour + dawnSpanHour)
        {
            float tt = (hour - (sunriseHour - dawnSpanHour)) / (dawnSpanHour * 2.0);
            sunVisibility = clamp(tt, 0.0, 1.0);
        }
        // 日中はフル
        else if (hour >= sunriseHour + dawnSpanHour && hour <= sunsetHour - duskSpanHour)
        {
            sunVisibility = 1.0;
        }
        // 日の入り前後フェードアウト
        else if (hour > sunsetHour - duskSpanHour && hour <= sunsetHour + duskSpanHour)
        {
            float tt = (sunsetHour + duskSpanHour - hour) / (duskSpanHour * 2.0);
            sunVisibility = clamp(tt, 0.0, 1.0);
        }

        if (uWeatherType <= 1 && sunVisibility > 0.001)
        {
            float sunAmount = clamp(dot(dir, -normalize(uSunDir)), 0.0, 1.0);

            float sunCore  = pow(sunAmount, 4096.0);
            float sunHalo  = pow(sunAmount, 100.0);
            
            vec3 sunCoreColor = vec3(1.3, 1.1, 0.8);
            vec3 sunHaloColor = vec3(1.1, 0.9, 0.7);
            vec3 sunGlow = sunCoreColor * sunCore + sunHaloColor * sunHalo;

            float cloudFactor = 1.0 - cloudAlpha;
            float weatherDim = (uWeatherType == 1) ? 0.6 : 1.0;

            sunGlow *= sunVisibility * weatherDim * cloudFactor;

            finalColor += sunGlow;
        }
    }

    FragColor = vec4(finalColor, 1.0);
}

