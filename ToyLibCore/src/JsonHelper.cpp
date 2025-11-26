#include "JsonHelper.h"

namespace JsonHelper
{
    bool GetInt(const nlohmann::json& obj, const char* key, int& out)
    {
        if (obj.contains(key) && obj[key].is_number_integer())
        {
            out = obj[key].get<int>();
            return true;
        }
        return false;
    }

    bool GetFloat(const nlohmann::json& obj, const char* key, float& out)
    {
        if (obj.contains(key) && obj[key].is_number())
        {
            out = obj[key].get<float>();
            return true;
        }
        return false;
    }

    bool GetBool(const nlohmann::json& obj, const char* key, bool& out)
    {
        if (obj.contains(key) && obj[key].is_boolean())
        {
            out = obj[key].get<bool>();
            return true;
        }
        return false;
    }

    bool GetString(const nlohmann::json& obj, const char* key, std::string& out)
    {
        if (obj.contains(key) && obj[key].is_string())
        {
            out = obj[key].get<std::string>();  // 明示的にgetを使って型を変換
            return true;
        }
        return false;
    }
    // Vector2
    bool GetVector2(const nlohmann::json& obj, const char* key, Vector2& out)
    {
        if (obj.contains(key))
        {
            const auto& jv = obj[key];
            out =  Vector2(
                           jv[0].get<float>(),
                           jv[1].get<float>()
                           );
            return true;
        }
        return false;
    }

    // Vector3
    bool GetVector3(const nlohmann::json& obj, const char* key, Vector3& out)
    {
        if (obj.contains(key) && obj[key].is_array() && obj[key].size() == 3)
        {
            const auto& jv = obj[key];
            out = Vector3(
                jv[0].get<float>(),  // x
                jv[1].get<float>(),  // y
                jv[2].get<float>()   // z
            );
            return true;
        }
        return false;
    }

    bool GetQuaternionFromEuler(const nlohmann::json& obj, const char* key, Quaternion& out)
    {
        if (obj.contains(key) && obj[key].is_array() && obj[key].size() == 3)
        {
            const auto& euler = obj[key];

            // 各軸の角度を取得
            float pitch = euler[0].get<float>(); // X軸 (ピッチ)
            float yaw   = euler[1].get<float>(); // Y軸 (ヨー)
            float roll  = euler[2].get<float>(); // Z軸 (ロール)

            // ラジアンに変換
            pitch = Math::ToRadians(pitch);
            yaw   = Math::ToRadians(yaw);
            roll  = Math::ToRadians(roll);

            // 各軸ごとのクォータニオンを作成
            Quaternion qx(Vector3::UnitX, pitch); // ピッチ
            Quaternion qy(Vector3::UnitY, yaw);   // ヨー
            Quaternion qz(Vector3::UnitZ, roll);  // ロール

            // クォータニオンの結合 (順番が重要)
            out =  Quaternion::Concatenate(qz, Quaternion::Concatenate(qy, qx));
            return true;
        }
        return false;
    }
}
