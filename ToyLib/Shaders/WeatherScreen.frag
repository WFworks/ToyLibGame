#version 410 core

//==================================================
// WeatherScreen.frag
// 画面全体に重ねる「前景エフェクト」用シェーダ
// - 雨スジ
// - 雪
// - もやっとした前景フォグ
//==================================================

out vec4 FragColor;

//------------------------------
// Uniforms
//------------------------------
uniform float uTime;          // 経過時間（アニメーション用）
uniform vec2  uResolution;    // 画面サイズ
uniform float uRainAmount;    // 雨の強さ  0.0〜1.0
uniform float uSnowAmount;    // 雪の強さ  0.0〜1.0
uniform float uFogAmount;     // フォグの強さ 0.0〜1.0

// 雪の粒の数
const int SNOW_COUNT = 80;

//==================================================
// 共通：ハッシュ＆ノイズ
//==================================================

//--- ハッシュ関数（float 版） ---
float hash(float x)
{
    return fract(sin(x) * 43758.5453123);
}

//--- ハッシュ関数（vec2 版） ---
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(27.619, 57.583))) * 43758.5453);
}

//--- 2D value noise ---
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(a, b, u.x),
               mix(c, d, u.x), u.y);
}

//--- fbm（Fractal Brownian Motion） ---
float fbm(vec2 p)
{
    float value = 0.0;
    float amp   = 0.5;
    
    for (int i = 0; i < 4; i++)
    {
        value += amp * noise(p);
        p     *= 2.0;
        amp   *= 0.5;
    }
    return value;
}

//==================================================
// 雨エフェクト
//==================================================
//
// 縦方向に伸びた「雨スジ」をランダムな x 位置に配置。
// y 方向に時間でスクロールさせて落ちているように見せる。
//--------------------------------------------------
float rainPattern(vec2 uv)
{
    // 横方向の密度を上げ（300）、縦だけ時間でスクロール
    uv *= vec2(300.0, 1.0);
    uv.y += uTime * 8.0;

    // 各スジごとの ID とオフセット
    float id     = floor(uv.x);
    float offset = hash(vec2(id, 0.0));

    // 0〜1 の範囲で繰り返す縦位置
    float y = fract(uv.y + offset);

    // 細くて尻尾のある形状
    float shape = smoothstep(0.0, 0.01, y) * (1.0 - y);

    return shape;
}

//==================================================
// 雪エフェクト
//==================================================
//
// SNOW_COUNT 個の粒をランダム配置し、時間で y を下に流す。
// 1 粒ごとにランダムな大きさ・速度を付与。
//--------------------------------------------------
float snowPattern(vec2 uv)
{
    float brightness = 0.0;

    for (int i = 0; i < SNOW_COUNT; i++)
    {
        float fi = float(i);

        // x 位置（わずかに左右に揺らす）
        float x = hash(fi * 1.3) + sin(uTime * 0.2 + fi) * 0.01;
        
        // 落下速度
        float speed = 0.1 + hash(fi * 3.2) * 0.5;

        // y は時間で下方向へスクロール（fract でループ）
        float y = fract(hash(fi * 2.1) - uTime * speed);

        vec2 snowPos = vec2(x, y);

        // uv からの距離で丸い粒にする
        float dist = length(uv - snowPos);

        // ランダムサイズ
        float size = 0.01 + hash(fi * 4.0) * 0.01;

        // 中心ほど明るい丸い粒
        brightness += smoothstep(size, 0.0, dist);
    }

    return brightness;
}

//==================================================
// 前景フォグエフェクト
//==================================================
//
// 画面中央を基準に、ノイズで「もやっ」とした濃淡を作る。
//--------------------------------------------------
float fogPattern(vec2 uv)
{
    // 画面中央原点・縦幅基準で正規化
    vec2 centeredUV = (gl_FragCoord.xy - 0.5 * uResolution) / uResolution.y;

    // ノイズ用のスケール
    vec2 noiseUV = centeredUV * 1.5;

    // ゆっくり流れる fbm ノイズ
    float n = fbm(noiseUV + vec2(0.0, uTime * 0.02));

    // ノイズ値をフォグ濃度にマッピング
    return smoothstep(0.3, 1.0, n);
}

//==================================================
// メイン
//==================================================
void main()
{
    // 0〜1 に正規化した画面座標
    vec2 uv = gl_FragCoord.xy / uResolution;

    // アルファの蓄積用
    float alpha = 0.0;

    // 雨の重ね合わせ
    if (uRainAmount > 0.01)
    {
        float rain = rainPattern(uv);
        alpha += rain * uRainAmount * 0.25; // 雨はやや控えめ
    }

    // 雪の重ね合わせ
    if (uSnowAmount > 0.01)
    {
        float snow = snowPattern(uv);
        alpha += snow * uSnowAmount * 1.2; // 雪は少し強め
    }

    // フォグの重ね合わせ
    if (uFogAmount > 0.01)
    {
        float fog = fogPattern(uv);
        alpha += fog * uFogAmount * 0.9;
    }

    // すべて白い前景エフェクトとして合成（色は vec3(1.0)）
    FragColor = vec4(vec3(1.0), clamp(alpha, 0.0, 1.0));
}
