#pragma once

#include <memory>
#include "Application.h"

// CreateUserApplication()
std::unique_ptr<Application> CreateUserApplication();

// Applicationクラス生成
#define TOYLIB_REGISTER_APP(AppType, ...)                   \
    std::unique_ptr<Application> CreateUserApplication() {  \
        return std::make_unique<AppType>(__VA_ARGS__);      \
    }
/*
#define TOYLIB_REGISTER_APP_WITH_ARGS(AppType, ...)         \
    std::unique_ptr<Application> CreateUserApplication() {  \
        return std::make_unique<AppType>(__VA_ARGS__);      \
    }
*/
