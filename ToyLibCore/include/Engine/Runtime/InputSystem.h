#pragma once

#include "Utils/MathUtil.h"
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_gamecontroller.h>

#include <string>
#include <vector>
#include <unordered_map>

// ボタンステータスの定義
enum ButtonState
{
    ENone,
    EPressed,
    EReleased,
    EHeld
};

//======================
// アクションバインディング
//======================

struct KeyboardBinding
{
    SDL_Scancode Scancode = SDL_SCANCODE_UNKNOWN;
};

struct ActionBinding
{
    std::string Name;                          // "Jump" など
    std::vector<KeyboardBinding> KeyBindings;  // 複数キー対応
};


// キーボード用
class KeyboardState
{
public:
    friend class InputSystem;
    bool GetKeyValue(SDL_Scancode keyCode) const;
    ButtonState GetKeyState(SDL_Scancode keyCode) const;
private:
    const Uint8* mCurrState;
    Uint8 mPrevState[SDL_NUM_SCANCODES];
};

// コントローラー用（現状そのまま）
class ControllerState
{
public:
    friend class InputSystem;

    bool GetButtonValue(SDL_GameControllerButton button) const;
    ButtonState GetButtonState(SDL_GameControllerButton button) const;

    const Vector2& GetLeftStick() const { return mLeftStick; }
    const Vector2& GetRightStick() const { return mRightStick; }
    float GetLeftTrigger() const { return mLeftTrigger; }
    float GetRightTrigger() const { return mRightTrigger; }

    bool GetIsConnected() const { return mIsConnected; }
private:
    Uint8 mCurrButtons[SDL_CONTROLLER_BUTTON_MAX];
    Uint8 mPrevButtons[SDL_CONTROLLER_BUTTON_MAX];
    Vector2 mLeftStick;
    Vector2 mRightStick;
    float mLeftTrigger;
    float mRightTrigger;
    bool mIsConnected;
};


// 入力情報のラッパー
struct InputState
{
    KeyboardState Keyboard;
    ControllerState Controller;
};


// 入力システム（キーボードのみ実装）
class InputSystem
{
public:
    bool Initialize();
    void Shutdown();

    void PrepareForUpdate();
    void Update();
    
    const InputState& GetState() const { return mState; }

    //=======================
    // 追加したい高レベルAPI
    //=======================

    // JSONからアクション設定を読み込む
    bool LoadActionConfig(const std::string& filePath);

    // アクション状態問い合わせ（キーボードのみ）
    bool IsActionDown(const std::string& actionName) const;     // Held or Pressed
    bool IsActionPressed(const std::string& actionName) const;  // このフレームで押された
    bool IsActionReleased(const std::string& actionName) const; // このフレームで離された
    
    
private:
    float Filter1D(int input);
    Vector2 Filter2D(int inputX, int inputY);

    SDL_GameController* mController;
    InputState mState;

    // アクション名 → バインディング
    std::unordered_map<std::string, ActionBinding> mActionBindings;
};
