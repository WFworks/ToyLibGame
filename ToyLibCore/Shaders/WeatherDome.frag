#version 410 core

out vec4 FragColor;
in vec3 vWorldDir;

uniform float uTime;
uniform int uWeatherType;     // 0: Clear, 1: Cloudy, 2: Rain, 3: Storm, 4: Snow
uniform float uTimeOfDay;     // 0.0〜1.0（夜→昼→夜）
uniform vec3 uSunDir;         // 太陽ベクトル（ワールド空間）


uniform vec3 uRawSkyColor;
uniform vec3 uRawCloudColor;

// 高速・安定な2Dハッシュ（sin不使用）
float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.x, p.y, p.x) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Value noise（バイリニア補間）
float vnoise(vec2 p)
{
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

float fbm(vec2 p)
{
    float value = 0.0;
    float amp   = 0.5;
    // pが巨大化して精度落ちしないように、各オクターブでwrap
    for (int i = 0; i < 5; i++)
    {
        value += amp * vnoise(p);
        p = (p * 2.0);
        p = mod(p, 1024.0); // ★ ここ重要：範囲を畳み込んでsin問題を根絶
        amp *= 0.5;
    }
    return value;
}
/*
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
*/
void main()
{
    float t = clamp(vWorldDir.y, 0.0, 1.0);
    float weatherFade = (uWeatherType == 0) ? 1.0 : 0.3;

    //vec3 rawSky   = getSkyColor(uTimeOfDay);
    //vec3 rawCloud = getCloudColor(uTimeOfDay);
    vec3 rawSky   = uRawSkyColor;
    vec3 rawCloud = uRawCloudColor;
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
        /*
        float sunAmount = clamp(dot(normalize(vWorldDir), -normalize(uSunDir)), 0.0, 1.0);
        vec3 sunGlow = vec3(1.2, 1.0, 0.8) * pow(sunAmount, 512.0);
        finalColor += sunGlow * (1.0 - cloudAlpha); // 雲が薄いほど強く見える
         */
        
        
        // --- 太陽ハイライト（時間＋雲＋天気で調整） ---
        // uTimeOfDay: 0.0〜1.0 = 0:00〜24:00 前提
        float hour = uTimeOfDay * 24.0;

        const float sunriseHour  = 5.0;   // 日の出
        const float sunsetHour   = 18.0;  // 日の入り
        const float dawnSpanHour = 1.0;   // 日の出前後フェード幅
        const float duskSpanHour = 1.0;   // 日の入り前後フェード幅

        float sunVisibility = 0.0;

        // 日の出前後フェードイン
        if (hour >= sunriseHour - dawnSpanHour && hour < sunriseHour + dawnSpanHour)
        {
            float t = (hour - (sunriseHour - dawnSpanHour)) / (dawnSpanHour * 2.0);
            sunVisibility = clamp(t, 0.0, 1.0);
        }
        // 日中はフル
        else if (hour >= sunriseHour + dawnSpanHour && hour <= sunsetHour - duskSpanHour)
        {
            sunVisibility = 1.0;
        }
        // 日の入り前後フェードアウト
        else if (hour > sunsetHour - duskSpanHour && hour <= sunsetHour + duskSpanHour)
        {
            float t = (sunsetHour + duskSpanHour - hour) / (duskSpanHour * 2.0);
            sunVisibility = clamp(t, 0.0, 1.0);
        }
        // それ以外（夜）は 0 のまま
        
        // 天気と雲も考慮（CLEAR/CLOUDY のときだけ見える）
        if (uWeatherType <= 1 && sunVisibility > 0.001)
        {
            float sunAmount = clamp(dot(normalize(vWorldDir), -normalize(uSunDir)), 0.0, 1.0);

            // コア＋ハローで少しリッチに
            float sunCore  = pow(sunAmount, 5120.0);
            float sunHalo  = pow(sunAmount, 1024.0);
            
            vec3 sunCoreColor = vec3(1.3, 1.1, 0.8);
            vec3 sunHaloColor = vec3(1.1, 0.9, 0.7);
            vec3 sunGlow = sunCoreColor * sunCore + sunHaloColor * sunHalo;

            float cloudFactor = (1.0 - cloudAlpha);
            float weatherDim = (uWeatherType == 1) ? 0.6 : 1.0; // CLOUDY はちょい弱め

            sunGlow *= sunVisibility * weatherDim * cloudFactor;

            finalColor += sunGlow;
        }
    }

    FragColor = vec4(finalColor, 1.0);
}
