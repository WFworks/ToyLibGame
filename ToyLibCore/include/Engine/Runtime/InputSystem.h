#pragma once

#include "Utils/MathUtil.h"
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_gamecontroller.h>

#include <string>
#include <vector>
#include <array>
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
// 論理ボタン（ゲーム用）
//======================
// 「Aボタンをジャンプにするか攻撃にするか」はゲーム側で決める。
// ここでは「どの入力を取れるか」だけを決める。
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
    Select,    // Stop 的に使ってもいい
    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,

    KeyW,      // キーボード WASD
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
    std::vector<SDL_Scancode> Keyboard;               // キーボード
    std::vector<SDL_GameControllerButton> Gamepad;    // パッドボタン
    // 将来トリガー(L2/R2)をアナログ扱いするなら別枠を増やせる
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
    Uint8 mPrevState[SDL_NUM_SCANCODES]{};
};

// コントローラー用
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
    Uint8 mCurrButtons[SDL_CONTROLLER_BUTTON_MAX]{};
    Uint8 mPrevButtons[SDL_CONTROLLER_BUTTON_MAX]{};
    Vector2 mLeftStick = Vector2::Zero;
    Vector2 mRightStick = Vector2::Zero;
    float mLeftTrigger = 0.0f;
    float mRightTrigger = 0.0f;
    bool mIsConnected = false;
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
    // InputSystem への弱い参照（生ポインタでOK）
    const class InputSystem* mOwner = nullptr;

};

//======================
// 入力システム
//======================

class InputSystem
{
public:
    bool Initialize();
    void Shutdown();

    // SDL_PollEvents前に呼ぶ（前フレームの状態を保存）
    void PrepareForUpdate();
    // SDL_PollEventsの後に呼ぶ（現在の状態を取得）
    void Update();
    
    // 低レベル状態
    const InputState& GetState() const { return mState; }

    //=======================
    // ボタンコンフィグ
    //=======================

    // JSONから論理ボタンごとの設定を読み込む
    // - filePath: 例 "GameApp/Settings/InputConfig.json"
    bool LoadButtonConfig(const std::string& filePath);

    //=======================
    // 論理ボタン問い合わせAPI
    //=======================

    bool IsButtonDown(GameButton button) const;      // Held or Pressed
    bool IsButtonPressed(GameButton button) const;   // このフレームで押された
    bool IsButtonReleased(GameButton button) const;  // このフレームで離された
    
    
    void SetTextInputMode(bool enabled);
    bool IsTextInputMode() const { return mTextInputMode; }

private:
    float   Filter1D(int input);
    Vector2 Filter2D(int inputX, int inputY);

    SDL_GameController* mController = nullptr;
    InputState mState;

    // 論理ボタン → 実キー/ボタン
    std::array<ButtonBinding, static_cast<size_t>(GameButton::Count)> mButtonBindings;
    
    // テキスト入力を受け付けるか
    bool mTextInputMode = false;
};


/*
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
*/
