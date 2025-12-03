#pragma once

#include <memory>
#include "Engine/Core/Application.h"

// CreateUserApplication()
std::unique_ptr<toy::Application> CreateUserApplication();



// Applicationクラス生成

#define TOYLIB_REGISTER_APP(AppType, ...)                   \
    std::unique_ptr<toy::Application> CreateUserApplication() {  \
        return std::make_unique<AppType>(__VA_ARGS__);      \
    }

/*
#define TOYLIB_REGISTER_APP(AppType, ...)                           \
    std::unique_ptr<toy::Application> CreateUserApplication() {     \
        return std::unique_ptr<toy::Application>(                   \
            static_cast<toy::Application*>(                         \
                new AppType(__VA_ARGS__)                            \
            )                                                       \
        );                                                          \
    }
*/
