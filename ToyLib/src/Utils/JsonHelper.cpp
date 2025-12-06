#include "Utils/JsonHelper.h"
#include <iostream>
#include <fstream>

namespace JsonHelper
{
    //==========================================================================
    // 基本型
    //==========================================================================

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
            // 明示的に get<std::string>() で型変換
            out = obj[key].get<std::string>();
            return true;
        }
        return false;
    }

    //==========================================================================
    // 文字列配列
    //==========================================================================

    bool GetStringArray(const nlohmann::json& obj,
                        const char* key,
                        std::vector<std::string>& out)
    {
        if (!obj.contains(key) || !obj[key].is_array())
        {
            return false;
        }

        const auto& arr = obj[key];
        out.clear();
        out.reserve(arr.size());

        for (const auto& v : arr)
        {
            if (v.is_string())
            {
                out.emplace_back(v.get<std::string>());
            }
        }

        // 有効な文字列が 1 つもなければ false
        return !out.empty();
    }

    //==========================================================================
    // Vector 系
    //==========================================================================

    bool GetVector2(const nlohmann::json& obj, const char* key, Vector2& out)
    {
        // ※ 配列長や型チェックは最小限。JSON フォーマット前提で使う想定。
        if (obj.contains(key))
        {
            const auto& jv = obj[key];
            out = Vector2(
                jv[0].get<float>(),
                jv[1].get<float>()
            );
            return true;
        }
        return false;
    }

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

    //==========================================================================
    // Euler → Quaternion
    //==========================================================================

    bool GetQuaternionFromEuler(const nlohmann::json& obj,
                                const char* key,
                                Quaternion& out)
    {
        if (obj.contains(key) && obj[key].is_array() && obj[key].size() == 3)
        {
            const auto& euler = obj[key];

            // 各軸の角度を取得（単位：度）
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

            // クォータニオンの結合（回転順はここで統一）
            // out = qz * (qy * qx) 的なイメージ
            out = Quaternion::Concatenate(qz, Quaternion::Concatenate(qy, qx));
            return true;
        }
        return false;
    }

    //==========================================================================
    // JSONファイル I/O
    //==========================================================================

    bool LoadFromFile(const std::string& path, nlohmann::json& out)
    {
        std::ifstream ifs(path);
        if (!ifs)
        {
            std::cerr << "[JsonHelper] Failed to open json file: " << path << std::endl;
            return false;
        }

        try
        {
            ifs >> out;
        }
        catch (const std::exception& e)
        {
            std::cerr << "[JsonHelper] JSON parse error in " << path
                      << ": " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    //==========================================================================
    // サブオブジェクト取得
    //==========================================================================

    bool GetObject(const nlohmann::json& obj, const char* key, nlohmann::json& out)
    {
        if (obj.contains(key) && obj[key].is_object())
        {
            out = obj[key];
            return true;
        }
        return false;
    }

} // namespace JsonHelper
