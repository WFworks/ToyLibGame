#pragma once

#include <memory>
#include "Engine/Core/Application.h"

//-------------------------------------------------------------
// CreateUserApplication()
// ・ユーザー側（ゲーム側）が定義する Application 派生クラスを生成するための関数。
// ・メイン関数（ToyLib 内部）がこの関数を呼び、ユーザーアプリを起動する。
// ・TOYLIB_REGISTER_APP マクロを使うことで自動的に実装される。
//-------------------------------------------------------------
std::unique_ptr<toy::Application> CreateUserApplication();


//-------------------------------------------------------------
// TOYLIB_REGISTER_APP(AppType, ...)
// ・ユーザーアプリ（Application 派生クラス）を登録するマクロ。
// ・AppType のコンストラクタ引数をそのまま渡せる。
// ・実体となる CreateUserApplication() を自動実装する。
//
// 【使用例】
//     TOYLIB_REGISTER_APP(MyGameApp, "My Title", 1280, 720)
//
// これにより、ToyLib 内の main() から MyGameApp が生成されて実行される。
//-------------------------------------------------------------
#define TOYLIB_REGISTER_APP(AppType, ...)                          \
    std::unique_ptr<toy::Application> CreateUserApplication() {    \
        return std::make_unique<AppType>(__VA_ARGS__);             \
    }
