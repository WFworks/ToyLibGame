#pragma once

#include "MathUtils.h"
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_gamecontroller.h>
//#include <SDL2/SDL_mouse.h>

// ボタンステータスの定義
enum ButtonState
{
    ENone,
    EPressed,
    EReleased,
    EHeld
};


// キーボード用
class KeyboardState
{
public:
    // InputSystemにフレンド
    friend class InputSystem;
    // boolで応答
    bool GetKeyValue(SDL_Scancode keyCode) const;
    // ステータスで応答
    ButtonState GetKeyState(SDL_Scancode keyCode) const;
private:
    const Uint8* mCurrState;
    Uint8 mPrevState[SDL_NUM_SCANCODES];
};

// コントローラー用
class ControllerState
{
public:
    friend class InputSystem;

    // ボタン
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
    // 左右のスティック
    Vector2 mLeftStick;
    Vector2 mRightStick;
    // 左右のトリガー
    float mLeftTrigger;
    float mRightTrigger;
    // 接続されているか
    bool mIsConnected;
};



// 入力情報のラッパー
// マウス、ジョイコントローラーなどもここでラップ
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

    // SDL_PollEvents前に呼ぶ、状態を記録
    void PrepareForUpdate();
    // SDL_PollEventsの後に呼ぶ
    void Update();
    
    // 状態を返す
    const InputState& GetState() const { return mState; }

private:
    float Filter1D(int input);
    Vector2 Filter2D(int inputX, int inputY);
    SDL_GameController* mController;
    InputState mState;
    

};


