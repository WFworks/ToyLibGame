#pragma once

#include "Utils/MathUtil.h"

#include <SDL3/SDL.h>   // SDL3 はこれ1つでOK
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

namespace toy {

// ボタンステータスの定義
enum ButtonState
{
    ENone,
    EPressed,
    EReleased,
    EHeld
};

//======================
// 論理ボタン（ゲーム用）
//======================
enum class GameButton
{
    A,
    B,
    X,
    Y,
    L1,
    L2,
    R1,
    R2,
    Start,
    Select,
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,

    KeyW,
    KeyA,
    KeyS,
    KeyD,

    Count
};

//======================
// バインディング構造体
//======================

struct ButtonBinding
{
    std::vector<SDL_Scancode>      Keyboard; // キーボード
    std::vector<SDL_GamepadButton> Gamepad;  // パッドボタン(SDL3)
};

//======================
// 低レベル入力
//======================

// キーボード用
class KeyboardState
{
public:
    friend class InputSystem;
    bool GetKeyValue(SDL_Scancode keyCode) const;
    ButtonState GetKeyState(SDL_Scancode keyCode) const;

private:
    const Uint8* mCurrState = nullptr;
    Uint8        mPrevState[SDL_SCANCODE_COUNT]{};
};

// コントローラー用（SDL3: Gamepad）
class ControllerState
{
public:
    friend class InputSystem;

    bool GetButtonValue(SDL_GamepadButton button) const;
    ButtonState GetButtonState(SDL_GamepadButton button) const;

    const Vector2& GetLeftStick()  const { return mLeftStick;  }
    const Vector2& GetRightStick() const { return mRightStick; }
    float GetLeftTrigger()  const { return mLeftTrigger;  }
    float GetRightTrigger() const { return mRightTrigger; }

    bool GetIsConnected() const { return mIsConnected; }

private:
    Uint8   mCurrButtons[SDL_GAMEPAD_BUTTON_COUNT]{};
    Uint8   mPrevButtons[SDL_GAMEPAD_BUTTON_COUNT]{};
    Vector2 mLeftStick  = Vector2::Zero;
    Vector2 mRightStick = Vector2::Zero;
    float   mLeftTrigger  = 0.0f;
    float   mRightTrigger = 0.0f;
    bool    mIsConnected  = false;
};

// 入力情報のラッパー
struct InputState
{
    KeyboardState   Keyboard;
    ControllerState Controller;

    bool IsButtonDown(GameButton button) const;
    bool IsButtonPressed(GameButton button) const;
    bool IsButtonReleased(GameButton button) const;

    void SetOwner(class InputSystem* inputSystem);

private:
    const class InputSystem* mOwner = nullptr;
};

//======================
// 入力システム
//======================

class InputSystem
{
public:
    bool Initialize(SDL_Window* window);
    void Shutdown();

    void PrepareForUpdate(); // SDL_PollEvents 前
    void Update();           // SDL_PollEvents 後

    const InputState& GetState() const { return mState; }

    // JSON からバインディング読み込み
    bool LoadButtonConfig(const std::string& filePath);

    // 論理ボタン問い合わせAPI
    bool IsButtonDown(GameButton button) const;
    bool IsButtonPressed(GameButton button) const;
    bool IsButtonReleased(GameButton button) const;

    void SetTextInputMode(bool enabled);
    bool IsTextInputMode() const { return mTextInputMode; }

private:
    float   Filter1D(int input);
    Vector2 Filter2D(int inputX, int inputY);

    SDL_Gamepad* mGamepad = nullptr; // SDL3: Gamepad
    InputState   mState;

    std::array<ButtonBinding, static_cast<size_t>(GameButton::Count)> mButtonBindings;

    bool mTextInputMode = false;
    
    SDL_Window* mWindow = nullptr;
};

} // namespace toy
