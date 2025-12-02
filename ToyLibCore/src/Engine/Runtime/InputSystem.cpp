#include "Engine/Runtime/InputSystem.h"
#include "Utils/JsonHelper.h"

#include <SDL2/SDL.h>
#include <cstring>
#include <fstream>
#include <iostream>


//--------------------------------------
// 文字列 → SDL_Scancode
//--------------------------------------
static SDL_Scancode StringToScancode(const std::string& name)
{
    static const std::unordered_map<std::string, SDL_Scancode> table = {
        // アルファベット
        {"A", SDL_SCANCODE_A}, {"B", SDL_SCANCODE_B}, {"C", SDL_SCANCODE_C},
        {"D", SDL_SCANCODE_D}, {"E", SDL_SCANCODE_E}, {"F", SDL_SCANCODE_F},
        {"G", SDL_SCANCODE_G}, {"H", SDL_SCANCODE_H}, {"I", SDL_SCANCODE_I},
        {"J", SDL_SCANCODE_J}, {"K", SDL_SCANCODE_K}, {"L", SDL_SCANCODE_L},
        {"M", SDL_SCANCODE_M}, {"N", SDL_SCANCODE_N}, {"O", SDL_SCANCODE_O},
        {"P", SDL_SCANCODE_P}, {"Q", SDL_SCANCODE_Q}, {"R", SDL_SCANCODE_R},
        {"S", SDL_SCANCODE_S}, {"T", SDL_SCANCODE_T}, {"U", SDL_SCANCODE_U},
        {"V", SDL_SCANCODE_V}, {"W", SDL_SCANCODE_W}, {"X", SDL_SCANCODE_X},
        {"Y", SDL_SCANCODE_Y}, {"Z", SDL_SCANCODE_Z},

        // 数字キー
        {"Num0", SDL_SCANCODE_0}, {"Num1", SDL_SCANCODE_1},
        {"Num2", SDL_SCANCODE_2}, {"Num3", SDL_SCANCODE_3},
        {"Num4", SDL_SCANCODE_4}, {"Num5", SDL_SCANCODE_5},
        {"Num6", SDL_SCANCODE_6}, {"Num7", SDL_SCANCODE_7},
        {"Num8", SDL_SCANCODE_8}, {"Num9", SDL_SCANCODE_9},

        // 特殊キー
        {"Space",   SDL_SCANCODE_SPACE},
        {"Escape",  SDL_SCANCODE_ESCAPE},
        {"Enter",   SDL_SCANCODE_RETURN},
        {"Tab",     SDL_SCANCODE_TAB},
        {"LeftShift",  SDL_SCANCODE_LSHIFT},
        {"RightShift", SDL_SCANCODE_RSHIFT},
        {"LeftCtrl",   SDL_SCANCODE_LCTRL},
        {"RightCtrl",  SDL_SCANCODE_RCTRL},

        // 矢印
        {"Up",    SDL_SCANCODE_UP},
        {"Down",  SDL_SCANCODE_DOWN},
        {"Left",  SDL_SCANCODE_LEFT},
        {"Right", SDL_SCANCODE_RIGHT},
    };

    auto it = table.find(name);
    if (it != table.end())
    {
        return it->second;
    }

    std::cerr << "[InputSystem] Unknown key name in config: " << name << std::endl;
    return SDL_SCANCODE_UNKNOWN;
}

//--------------------------------------
// 文字列 → SDL_GameControllerButton
//--------------------------------------
static SDL_GameControllerButton StringToPadButton(const std::string& name)
{
    static const std::unordered_map<std::string, SDL_GameControllerButton> table = {
        {"A", SDL_CONTROLLER_BUTTON_A},
        {"B", SDL_CONTROLLER_BUTTON_B},
        {"X", SDL_CONTROLLER_BUTTON_X},
        {"Y", SDL_CONTROLLER_BUTTON_Y},

        {"LeftShoulder",  SDL_CONTROLLER_BUTTON_LEFTSHOULDER},  // L1
        {"RightShoulder", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER}, // R1

        {"Start", SDL_CONTROLLER_BUTTON_START},
        {"Back",  SDL_CONTROLLER_BUTTON_BACK},

        {"DPadUp",    SDL_CONTROLLER_BUTTON_DPAD_UP},
        {"DPadDown",  SDL_CONTROLLER_BUTTON_DPAD_DOWN},
        {"DPadLeft",  SDL_CONTROLLER_BUTTON_DPAD_LEFT},
        {"DPadRight", SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
    };

    auto it = table.find(name);
    if (it != table.end())
    {
        return it->second;
    }

    std::cerr << "[InputSystem] Unknown gamepad button name in config: "
              << name << std::endl;
    return SDL_CONTROLLER_BUTTON_INVALID;
}

//--------------------------------------
// 文字列 → GameButton
//--------------------------------------
static bool StringToGameButton(const std::string& name, GameButton& out)
{
    static const std::unordered_map<std::string, GameButton> table = {
        {"A", GameButton::A},
        {"B", GameButton::B},
        {"X", GameButton::X},
        {"Y", GameButton::Y},
        {"L1", GameButton::L1},
        {"L2", GameButton::L2},
        {"R1", GameButton::R1},
        {"R2", GameButton::R2},
        {"Start",  GameButton::Start},
        {"Select", GameButton::Select},
        {"DPadUp",    GameButton::DPadUp},
        {"DPadDown",  GameButton::DPadDown},
        {"DPadLeft",  GameButton::DPadLeft},
        {"DPadRight", GameButton::DPadRight},
        {"KeyW", GameButton::KeyW},
        {"KeyA", GameButton::KeyA},
        {"KeyS", GameButton::KeyS},
        {"KeyD", GameButton::KeyD},
    };

    auto it = table.find(name);
    if (it == table.end())
    {
        return false;
    }
    out = it->second;
    return true;
}


//--------------------------------------
// KeyboardState
//--------------------------------------

bool KeyboardState::GetKeyValue(SDL_Scancode keyCode) const
{
    return mCurrState[keyCode] == 1;
}

// キーボードの状態を返す
ButtonState KeyboardState::GetKeyState(SDL_Scancode keyCode) const
{
    if (mPrevState[keyCode] == 0)
    {
        if (mCurrState[keyCode] == 0)
        {
            return ENone;
        }
        else
        {
            return EPressed;
        }
    }
    else // 前の状態が1の時
    {
        if (mCurrState[keyCode] == 0)
        {
            return EReleased;
        }
        else
        {
            return EHeld;
        }
    }
}


//--------------------------------------
// ControllerState
//--------------------------------------

bool ControllerState::GetButtonValue(SDL_GameControllerButton button) const
{
    return mCurrButtons[button] == 1;
}

// コントローラーのボタンの状態を返す
ButtonState ControllerState::GetButtonState(SDL_GameControllerButton button) const
{
    if (mPrevButtons[button] == 0)
    {
        if (mCurrButtons[button] == 0)
        {
            return ENone;
        }
        else
        {
            return EPressed;
        }
    }
    else
    {
        if (mCurrButtons[button] == 0)
        {
            return EReleased;
        }
        else
        {
            return EHeld;
        }
    }
}



//-----
bool InputState::IsButtonDown(GameButton btn) const
{
    // 念のため nullptr チェック
    return mOwner && mOwner->IsButtonDown(btn);
}

bool InputState::IsButtonPressed(GameButton btn) const
{
    return mOwner && mOwner->IsButtonPressed(btn);
}

bool InputState::IsButtonReleased(GameButton btn) const
{
    return mOwner && mOwner->IsButtonReleased(btn);
}
void InputState::SetOwner(class InputSystem* inputSystem)
{
    mOwner = inputSystem;
}



//--------------------------------------
// InputSystem
//--------------------------------------

bool InputSystem::Initialize()
{
    // キーボード
    mState.Keyboard.mCurrState = SDL_GetKeyboardState(nullptr);
    std::memset(mState.Keyboard.mPrevState, 0, SDL_NUM_SCANCODES);
    
    // コントローラーを探す
    mController = SDL_GameControllerOpen(0);
    // コントローラーがあれば初期化
    mState.Controller.mIsConnected = (mController != nullptr);
    std::memset(mState.Controller.mCurrButtons, 0, SDL_CONTROLLER_BUTTON_MAX);
    std::memset(mState.Controller.mPrevButtons, 0, SDL_CONTROLLER_BUTTON_MAX);
    
    // 内部呼び出しようにInputSystemのポインタだけ記憶
    mState.SetOwner(this);
    
    // テキスト入力モードをオフ
    mTextInputMode = false;
    SDL_StopTextInput();
    
    // ボタンバインディングは LoadButtonConfig で設定される
    return true;
}

void InputSystem::Shutdown()
{
    // 必要なら mController を閉じるなど
    if (mController)
    {
        SDL_GameControllerClose(mController);
        mController = nullptr;
    }
}

// 前の状態をコピー
void InputSystem::PrepareForUpdate()
{
    // キーボード
    std::memcpy(mState.Keyboard.mPrevState,
                mState.Keyboard.mCurrState,
                SDL_NUM_SCANCODES);

    // コントローラー
    std::memcpy(mState.Controller.mPrevButtons,
                mState.Controller.mCurrButtons,
                SDL_CONTROLLER_BUTTON_MAX);
}

void InputSystem::Update()
{
    // キーボードは SDL が内部で更新してくれているので何もしない

    if (mState.Controller.mIsConnected && mController)
    {
        // ボタン
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
        {
            mState.Controller.mCurrButtons[i] =
                SDL_GameControllerGetButton(mController, static_cast<SDL_GameControllerButton>(i));
        }
        
        // トリガー（L2/R2 はとりあえずアナログ値で保持）
        mState.Controller.mLeftTrigger =
            Filter1D(SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
        mState.Controller.mRightTrigger =
            Filter1D(SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
        
        // スティック
        int x = 0, y = 0;
        x = SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_LEFTX);
        y = -SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_LEFTY);
        mState.Controller.mLeftStick = Filter2D(x, y);
        
        x = SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_RIGHTX);
        y = -SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_RIGHTY);
        mState.Controller.mRightStick = Filter2D(x, y);
    }
}


// デッドゾーンをフリップ
float InputSystem::Filter1D(int input)
{
    // A value < dead zone is interpreted as 0%
    const int deadZone = 250;
    // A value > max value is interpreted as 100%
    const int maxValue = 30000;

    float retVal = 0.0f;

    int absValue = input > 0 ? input : -input;
    if (absValue > deadZone)
    {
        retVal = static_cast<float>(absValue - deadZone) /
                 (maxValue - deadZone);
        retVal = input > 0 ? retVal : -retVal;
        retVal = Math::Clamp(retVal, -1.0f, 1.0f);
    }

    return retVal;
}

// デッドゾーンをフリップ
Vector2 InputSystem::Filter2D(int inputX, int inputY)
{
    const float deadZone = 8000.0f;
    const float maxValue = 30000.0f;

    Vector2 dir;
    dir.x = static_cast<float>(inputX);
    dir.y = static_cast<float>(inputY);

    float length = dir.Length();

    if (length < deadZone)
    {
        dir = Vector2::Zero;
    }
    else
    {
        float f = (length - deadZone) / (maxValue - deadZone);
        f = Math::Clamp(f, 0.0f, 1.0f);
        dir *= f / length;
    }

    return dir;
}


//--------------------------------------
// JSON からアクション設定を読み込み
//--------------------------------------

bool InputSystem::LoadButtonConfig(const std::string& filePath)
{
    std::ifstream ifs(filePath);
    if (!ifs)
    {
        std::cerr << "[InputSystem] Failed to open button config: "
                  << filePath << std::endl;
        return false;
    }

    nlohmann::json root;
    try
    {
        ifs >> root;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[InputSystem] JSON parse error in "
                  << filePath << ": " << e.what() << std::endl;
        return false;
    }

    if (!root.contains("buttons") || !root["buttons"].is_object())
    {
        std::cerr << "[InputSystem] 'buttons' object not found in: "
                  << filePath << std::endl;
        return false;
    }

    const auto& buttonsObj = root["buttons"];

    // 一旦クリア
    for (auto& b : mButtonBindings)
    {
        b.Keyboard.clear();
        b.Gamepad.clear();
    }

    for (auto it = buttonsObj.begin(); it != buttonsObj.end(); ++it)
    {
        const std::string buttonName = it.key();
        const nlohmann::json& buttonJson = it.value();

        GameButton gb;
        if (!StringToGameButton(buttonName, gb))
        {
            std::cerr << "[InputSystem] Unknown GameButton in config: "
                      << buttonName << std::endl;
            continue;
        }

        auto idx = static_cast<size_t>(gb);
        ButtonBinding& binding = mButtonBindings[idx];

        // キーボード配列
        std::vector<std::string> keyNames;
        if (JsonHelper::GetStringArray(buttonJson, "keyboard", keyNames))
        {
            for (const auto& keyName : keyNames)
            {
                SDL_Scancode sc = StringToScancode(keyName);
                if (sc != SDL_SCANCODE_UNKNOWN)
                {
                    binding.Keyboard.push_back(sc);
                }
            }
        }

        // ゲームパッド配列
        std::vector<std::string> padNames;
        if (JsonHelper::GetStringArray(buttonJson, "gamepad", padNames))
        {
            for (const auto& padName : padNames)
            {
                SDL_GameControllerButton b = StringToPadButton(padName);
                if (b != SDL_CONTROLLER_BUTTON_INVALID)
                {
                    binding.Gamepad.push_back(b);
                }
            }
        }
    }

    std::cout << "[InputSystem] Loaded button config: "
              << filePath << std::endl;

    return true;
}

//--------------------------------------
// 論理ボタン問い合わせ
//--------------------------------------

bool InputSystem::IsButtonDown(GameButton button) const
{
    auto idx = static_cast<size_t>(button);
    const ButtonBinding& binding = mButtonBindings[idx];

    // キーボード
    for (auto sc : binding.Keyboard)
    {
        ButtonState s = mState.Keyboard.GetKeyState(sc);
        if (s == EPressed || s == EHeld)
        {
            return true;
        }
    }

    // パッド
    for (auto pb : binding.Gamepad)
    {
        ButtonState s = mState.Controller.GetButtonState(pb);
        if (s == EPressed || s == EHeld)
        {
            return true;
        }
    }

    // ==============================
    // 右スティック → WASD マッピング
    // ==============================
    const Vector2& rs = mState.Controller.GetRightStick();
    const float threshold = 0.3f; // 好みで調整（0.2〜0.4くらい）

    switch (button)
    {
    case GameButton::KeyW:
        if (rs.y > threshold) return true;
        break;
    case GameButton::KeyS:
        if (rs.y < -threshold) return true;
        break;
    case GameButton::KeyD:
        if (rs.x > threshold) return true;
        break;
    case GameButton::KeyA:
        if (rs.x < -threshold) return true;
        break;
    default:
        break;
    }

    // L2/R2 をトリガーとして扱いたい場合はここで補正してもよい（TODO）
    return false;
}

bool InputSystem::IsButtonPressed(GameButton button) const
{
    auto idx = static_cast<size_t>(button);
    const ButtonBinding& binding = mButtonBindings[idx];

    for (auto sc : binding.Keyboard)
    {
        if (mState.Keyboard.GetKeyState(sc) == EPressed)
            return true;
    }
    for (auto pb : binding.Gamepad)
    {
        if (mState.Controller.GetButtonState(pb) == EPressed)
            return true;
    }
    return false;
}

bool InputSystem::IsButtonReleased(GameButton button) const
{
    auto idx = static_cast<size_t>(button);
    const ButtonBinding& binding = mButtonBindings[idx];

    for (auto sc : binding.Keyboard)
    {
        if (mState.Keyboard.GetKeyState(sc) == EReleased)
            return true;
    }
    for (auto pb : binding.Gamepad)
    {
        if (mState.Controller.GetButtonState(pb) == EReleased)
            return true;
    }
    return false;
}


void InputSystem::SetTextInputMode(bool enabled)
{
    if (enabled == mTextInputMode)
    {
        return; // 状態変わらないなら何もしない
    }

    mTextInputMode = enabled;

    if (enabled)
    {
        SDL_StartTextInput();
    }
    else
    {
        SDL_StopTextInput();
    }
}


/*
#include "Engine/Runtime/InputSystem.h"
#include "Utils/IMEUtil.h"
#include "Utils/JsonHelper.h"

#include <SDL2/SDL.h>
#include <cstring>

#include <fstream>
#include <iostream>

#include <fstream>
#include <iostream>

using json = nlohmann::json;

static SDL_Scancode StringToScancode(const std::string& name)
{
    static const std::unordered_map<std::string, SDL_Scancode> table = {
        // アルファベット
        {"A", SDL_SCANCODE_A}, {"B", SDL_SCANCODE_B}, {"C", SDL_SCANCODE_C},
        {"D", SDL_SCANCODE_D}, {"E", SDL_SCANCODE_E}, {"F", SDL_SCANCODE_F},
        {"G", SDL_SCANCODE_G}, {"H", SDL_SCANCODE_H}, {"I", SDL_SCANCODE_I},
        {"J", SDL_SCANCODE_J}, {"K", SDL_SCANCODE_K}, {"L", SDL_SCANCODE_L},
        {"M", SDL_SCANCODE_M}, {"N", SDL_SCANCODE_N}, {"O", SDL_SCANCODE_O},
        {"P", SDL_SCANCODE_P}, {"Q", SDL_SCANCODE_Q}, {"R", SDL_SCANCODE_R},
        {"S", SDL_SCANCODE_S}, {"T", SDL_SCANCODE_T}, {"U", SDL_SCANCODE_U},
        {"V", SDL_SCANCODE_V}, {"W", SDL_SCANCODE_W}, {"X", SDL_SCANCODE_X},
        {"Y", SDL_SCANCODE_Y}, {"Z", SDL_SCANCODE_Z},

        // 数字キー
        {"Num0", SDL_SCANCODE_0}, {"Num1", SDL_SCANCODE_1},
        {"Num2", SDL_SCANCODE_2}, {"Num3", SDL_SCANCODE_3},
        {"Num4", SDL_SCANCODE_4}, {"Num5", SDL_SCANCODE_5},
        {"Num6", SDL_SCANCODE_6}, {"Num7", SDL_SCANCODE_7},
        {"Num8", SDL_SCANCODE_8}, {"Num9", SDL_SCANCODE_9},

        // 特殊キー
        {"Space",   SDL_SCANCODE_SPACE},
        {"Escape",  SDL_SCANCODE_ESCAPE},
        {"Enter",   SDL_SCANCODE_RETURN},
        {"Tab",     SDL_SCANCODE_TAB},
        {"LeftShift",  SDL_SCANCODE_LSHIFT},
        {"RightShift", SDL_SCANCODE_RSHIFT},
        {"LeftCtrl",   SDL_SCANCODE_LCTRL},
        {"RightCtrl",  SDL_SCANCODE_RCTRL},

        // 矢印
        {"Up",    SDL_SCANCODE_UP},
        {"Down",  SDL_SCANCODE_DOWN},
        {"Left",  SDL_SCANCODE_LEFT},
        {"Right", SDL_SCANCODE_RIGHT},
    };

    auto it = table.find(name);
    if (it != table.end())
    {
        return it->second;
    }

    std::cerr << "[InputSystem] Unknown key name in config: " << name << std::endl;
    return SDL_SCANCODE_UNKNOWN;
}

bool KeyboardState::GetKeyValue(SDL_Scancode keyCode) const
{
    return mCurrState[keyCode] == 1;
}

// キーボードの状態を返す
ButtonState KeyboardState::GetKeyState(SDL_Scancode keyCode) const
{
    if (mPrevState[keyCode] == 0)
    {
        if (mCurrState[keyCode] == 0)
        {
            return ENone;
        }
        else
        {
            return EPressed;
        }
    }
    else // 前の状態が1の時
    {
        if (mCurrState[keyCode] == 0)
        {
            return EReleased;
        }
        else
        {
            return EHeld;
        }
    }
}



bool ControllerState::GetButtonValue(SDL_GameControllerButton button) const
{
    return mCurrButtons[button] == 1;
}

// コントローラーのボタンの状態を返す
ButtonState ControllerState::GetButtonState(SDL_GameControllerButton button) const
{
    if (mPrevButtons[button] == 0)
    {
        if (mCurrButtons[button] == 0)
        {
            return ENone;
        }
        else
        {
            return EPressed;
        }
    }
    else
    {
        if (mCurrButtons[button] == 0)
        {
            return EReleased;
        }
        else
        {
            return EHeld;
        }
    }
}



bool InputSystem::Initialize()
{
    // キーボード
    mState.Keyboard.mCurrState = SDL_GetKeyboardState(NULL);
    memset(mState.Keyboard.mPrevState, 0, SDL_NUM_SCANCODES);
    
    // マウス、コントローラーもここで初期化
    // コントローラーを探す
    mController = SDL_GameControllerOpen(0);
    // コントローラーがあれば初期化
    mState.Controller.mIsConnected = (mController != nullptr);
    memset(mState.Controller.mCurrButtons, 0, SDL_CONTROLLER_BUTTON_MAX);
    memset(mState.Controller.mPrevButtons, 0, SDL_CONTROLLER_BUTTON_MAX);
    
    return true;
}

void InputSystem::Shutdown()
{
}

// 前の状態をコピー
void InputSystem::PrepareForUpdate()
{
    // 前フレームの状態にコピー
    // キーボード
    memcpy(mState.Keyboard.mPrevState, mState.Keyboard.mCurrState, SDL_NUM_SCANCODES);
    // コントローラー
    memcpy(mState.Controller.mPrevButtons, mState.Controller.mCurrButtons, SDL_CONTROLLER_BUTTON_MAX);
}

void InputSystem::Update()
{
    // キーボードはアップデート必要なし
    
    
    // Controller
    // ボタン
//    if(state.Controller.isConnected)
    {
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
        {
            mState.Controller.mCurrButtons[i] = SDL_GameControllerGetButton(mController, SDL_GameControllerButton(i));
        }
        
        // トリガー
        mState.Controller.mLeftTrigger = Filter1D(SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
        mState.Controller.mRightTrigger = Filter1D(SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
        
        int x = 0, y = 0;
        // スティック
        x = SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_LEFTX);
        y = -SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_LEFTY);
        mState.Controller.mLeftStick = Filter2D(x, y);
        
        x = SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_RIGHTX);
        y = -SDL_GameControllerGetAxis(mController, SDL_CONTROLLER_AXIS_RIGHTY);
        mState.Controller.mRightStick = Filter2D(x, y);
    }
}


// デッドゾーンをフリップ
float InputSystem::Filter1D(int input)
{
    // A value < dead zone is interpreted as 0%
    const int deadZone = 250;
    // A value > max value is interpreted as 100%
    const int maxValue = 30000;

    float retVal = 0.0f;

    // Take absolute value of input
    int absValue = input > 0 ? input : -input;
    // Ignore input within dead zone
    if (absValue > deadZone)
    {
        // Compute fractional value between dead zone and max value
        retVal = static_cast<float>(absValue - deadZone) /
        (maxValue - deadZone);
        // Make sure sign matches original value
        retVal = input > 0 ? retVal : -1.0f * retVal;
        // Clamp between -1.0f and 1.0f
        retVal = Math::Clamp(retVal, -1.0f, 1.0f);
    }

    return retVal;
}

// デッドゾーンをフリップ
Vector2 InputSystem::Filter2D(int inputX, int inputY)
{
    const float deadZone = 8000.0f;
    const float maxValue = 30000.0f;

    // Make into 2D vector
    Vector2 dir;
    dir.x = static_cast<float>(inputX);
    dir.y = static_cast<float>(inputY);

    float length = dir.Length();

    // If length < deadZone, should be no input
    if (length < deadZone)
    {
        dir = Vector2::Zero;
    }
    else
    {
        // Calculate fractional value between
        // dead zone and max value circles
        float f = (length - deadZone) / (maxValue - deadZone);
        // Clamp f between 0.0f and 1.0f
        f = Math::Clamp(f, 0.0f, 1.0f);
        // Normalize the vector, and then scale it to the
        // fractional value
        dir *= f / length;
    }

    return dir;
}

bool InputSystem::LoadActionConfig(const std::string& filePath)
{
    std::ifstream ifs(filePath);
    if (!ifs)
    {
        std::cerr << "[InputSystem] Failed to open action config: "
                  << filePath << std::endl;
        return false;
    }

    json root;
    try
    {
        ifs >> root;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[InputSystem] JSON parse error: " << e.what() << std::endl;
        return false;
    }

    if (!root.contains("actions") || !root["actions"].is_object())
    {
        std::cerr << "[InputSystem] 'actions' object not found in: "
                  << filePath << std::endl;
        return false;
    }

    const auto& actionsObj = root["actions"];
    mActionBindings.clear();

    for (auto it = actionsObj.begin(); it != actionsObj.end(); ++it)
    {
        const std::string actionName = it.key();
        const json& actionJson = it.value();

        ActionBinding binding;
        binding.Name = actionName;

        // キーボードの配列をJsonHelperで取得
        std::vector<std::string> keyNames;
        if (JsonHelper::GetStringArray(actionJson, "keyboard", keyNames))
        {
            for (const auto& keyName : keyNames)
            {
                SDL_Scancode sc = StringToScancode(keyName);
                if (sc != SDL_SCANCODE_UNKNOWN)
                {
                    KeyboardBinding kb;
                    kb.Scancode = sc;
                    binding.KeyBindings.push_back(kb);
                }
            }
        }

        // 今はキーボードだけ。Gamepadは後で拡張する。

        if (!binding.KeyBindings.empty())
        {
            mActionBindings.emplace(binding.Name, std::move(binding));
        }
    }

    std::cout << "[InputSystem] Loaded action config: "
              << filePath << " (actions=" << mActionBindings.size() << ")"
              << std::endl;

    return true;
}

bool InputSystem::IsActionDown(const std::string& actionName) const
{
    auto it = mActionBindings.find(actionName);
    if (it == mActionBindings.end())
    {
        return false;
    }

    const auto& binding = it->second;
    for (const auto& kb : binding.KeyBindings)
    {
        ButtonState s = mState.Keyboard.GetKeyState(kb.Scancode);
        if (s == EHeld || s == EPressed)
        {
            return true;
        }
    }
    return false;
}

bool InputSystem::IsActionPressed(const std::string& actionName) const
{
    auto it = mActionBindings.find(actionName);
    if (it == mActionBindings.end())
    {
        return false;
    }

    const auto& binding = it->second;
    for (const auto& kb : binding.KeyBindings)
    {
        if (mState.Keyboard.GetKeyState(kb.Scancode) == EPressed)
        {
            return true;
        }
    }
    return false;
}

bool InputSystem::IsActionReleased(const std::string& actionName) const
{
    auto it = mActionBindings.find(actionName);
    if (it == mActionBindings.end())
    {
        return false;
    }

    const auto& binding = it->second;
    for (const auto& kb : binding.KeyBindings)
    {
        if (mState.Keyboard.GetKeyState(kb.Scancode) == EReleased)
        {
            return true;
        }
    }
    return false;
}
*/
