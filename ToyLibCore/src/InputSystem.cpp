#include "InputSystem.h"
#include "IMEUtil.h"

#include <SDL2/SDL.h>
#include <cstring>


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

