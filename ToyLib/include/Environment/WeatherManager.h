#pragma once
#include <memory>

namespace toy {

//======================================================================
// 天候タイプ
//======================================================================
// ゲーム全体で共通して扱う天気の列挙。
// SkyDome と WeatherOverlay の両方で参照される。
//======================================================================
enum class WeatherType
{
    CLEAR = 0,   // 快晴
    CLOUDY,      // 曇り
    RAIN,        // 雨
    STORM,       // 嵐（雷雨などを含む）
    SNOW         // 雪
};

//======================================================================
// 天候の詳細パラメータ（将来の拡張用）
//======================================================================
// ・雨量、霧の濃さ、降雪量など「強度」を保持できる構造体。
// ・現在は未使用でも、段階的な天候変化・複合天候に発展可能。
//======================================================================
struct WeatherState
{
    WeatherType type;
    float rainAmount;
    float fogAmount;
    float snowAmount;
};

//======================================================================
// WeatherManager
//======================================================================
// 天候全体を統括する管理クラス。
// ・WeatherDomeComponent … 空・太陽・フォグなどの環境表現
// ・WeatherOverlayComponent … 雨/雪などの画面効果
//
// これら2つをまとめて制御する「ハブ」として機能する。
//======================================================================
class WeatherManager
{
public:
    WeatherManager();

    //==================================================================
    // Update
    //==================================================================
    // 天候の時間変化・遷移処理（必要であれば）
    // ※現状では最低限の更新のみ
    //==================================================================
    void Update(float deltaTime);

    // SkyDome（空・光源）側との連携
    void SetWeatherDome(class WeatherDomeComponent* dome) { mWeatherDome = dome; }

    // Overlay（雨粒/雪/雷など）側との連携
    void SetWeatherOverlay(class WeatherOverlayComponent* overlay) { mWeatherOverlay = overlay; }

    //==================================================================
    // ChangeWeather
    //==================================================================
    // 天候を変更する。
    // ・SkyDome に天候をセット → 空の色や昼夜光源が変わる
    // ・Overlay に天候をセット → パーティクル強度が変わる
    //
    // 基本は即時反映。将来的にはフェード処理もここに追加可能。
    //==================================================================
    void ChangeWeather(const WeatherType weather);

private:
    // 画面エフェクト（雨・雪・雷など）
    class WeatherOverlayComponent* mWeatherOverlay = nullptr;

    // 空・光源・フォグなど
    class WeatherDomeComponent* mWeatherDome = nullptr;

    // 現在の天気（基準状態）
    WeatherType mWeather = WeatherType::CLEAR;
};

} // namespace toy
