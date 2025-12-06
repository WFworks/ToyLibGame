cmake_minimum_required(VERSION 3.15)
project(GameApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TOYLIB_PATH ToyLib)
set(TOYKIT_PATH ToyKit)
set(GAME_PATH  ToyGame)

find_package(OpenGL REQUIRED)

# ToyLib のソース
file(GLOB_RECURSE TOYLIB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/${TOYLIB_PATH}/src/*.cpp
)

# ToyKit のソース
file(GLOB_RECURSE TOYKIT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/${TOYKIT_PATH}/src/*.cpp
)

# GameApp 以下のソース（GameApp/RPG, GameApp/Shooting も全部拾う）
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/${GAME_PATH}/*.cpp
)

# まとめる
set(SOURCES
    ${TOYLIB_SOURCES}
    ${TOYKIT_SOURCES}
    ${GAME_SOURCES}
)

add_executable(${PROJECT_NAME} ${SOURCES})

# インクルードパス
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/${TOYLIB_PATH}/include
    ${CMAKE_SOURCE_DIR}/${TOYKIT_PATH}/include
    ${CMAKE_SOURCE_DIR}/${GAME_PATH}
)

#========================
# プラットフォーム別設定
#========================
if(WIN32)
    message(STATUS "Configuring for Windows build")

    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_image CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
    find_package(GLEW CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(assimp CONFIG REQUIRED)
    find_package(OpenAL CONFIG REQUIRED)  # openal-soft (vcpkg) 前提

    # ========================
    # vcpkg のルートとトリプレット
    # ========================
    # 必要なら自分の環境に合わせてここだけ変えてOK
    set(VCPKG_ROOT "C:/tools/vcpkg")
    set(VCPKG_TRIPLET "x64-windows")

    set(VCPKG_INC_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include")
    set(VCPKG_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib")
    set(VCPKG_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/bin")

    # ========================
    # mpg123 を手動で探す
    # ========================
    find_path(MPG123_INCLUDE_DIR
        NAMES mpg123.h
        HINTS ${VCPKG_INC_DIR}
    )

    find_library(MPG123_LIBRARY
        NAMES mpg123
        HINTS ${VCPKG_LIB_DIR}
    )

    if(NOT MPG123_INCLUDE_DIR OR NOT MPG123_LIBRARY)
        message(FATAL_ERROR "mpg123 not found. Run: vcpkg install mpg123:x64-windows")
    endif()

    # 既存の include に mpg123 を追加
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${VCPKG_INC_DIR}
        ${MPG123_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        OpenGL32
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        GLEW::GLEW
        assimp::assimp
        OpenAL::OpenAL
        ${MPG123_LIBRARY}      # ← ここを生 mpg123 じゃなくて .lib のフルパス変数に
    )

    # SDLのエントリポイント無効化
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

    # MSVC専用設定（Visual Studio系コンパイラ）
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /MP)
        target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:CONSOLE)
    endif()

    # ========================
    # DLL コピー（vcpkg/bin）
    # ========================

    set(DLL_FILES
        "${VCPKG_BIN_DIR}/SDL3.dll"
        "${VCPKG_BIN_DIR}/SDL3_image.dll"
        "${VCPKG_BIN_DIR}/jpeg62.dll"
        "${VCPKG_BIN_DIR}/libpng16.dll"
        "${VCPKG_BIN_DIR}/zlib1.dll"
        "${VCPKG_BIN_DIR}/SDL3_ttf.dll"
        "${VCPKG_BIN_DIR}/glew32.dll"
        "${VCPKG_BIN_DIR}/assimp-vc143-mt.dll"
        "${VCPKG_BIN_DIR}/fmt.dll"
        "${VCPKG_BIN_DIR}/freetype.dll"
        "${VCPKG_BIN_DIR}/bz2.dll"
        # OpenAL-soft 本体
        "${VCPKG_BIN_DIR}/openal32.dll"

        # BGM デコード用 mpg123
        "${VCPKG_BIN_DIR}/mpg123.dll"
    )

    foreach(dll ${DLL_FILES})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${dll}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying ${dll} to $<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach()

elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring for Linux build")

    find_package(OpenGL REQUIRED)

    # SDL3 系
    find_package(SDL3        REQUIRED)
    find_package(SDL3_image  REQUIRED)
    find_package(SDL3_ttf    REQUIRED)

    find_package(GLEW   REQUIRED)
    find_package(assimp REQUIRED)
    find_package(OpenAL REQUIRED)

    find_package(fmt CONFIG REQUIRED)

    # ========================
    # vcpkg のルートとトリプレット（必要なら使う）
    # ========================
    set(VCPKG_ROOT "$ENV{HOME}/Repos/vcpkg")
    set(VCPKG_TRIPLET "x64-linux")

    set(VCPKG_INC_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/include")
    set(VCPKG_LIB_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/lib")
    set(VCPKG_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}/bin")

    # ==== mpg123 を手動で探す ====
    find_path(MPG123_INCLUDE_DIR
        NAMES mpg123.h
        PATHS
            /usr/include
            /usr/local/include
            /usr/include/mpg123
            /usr/local/include/mpg123
    )

    find_library(MPG123_LIBRARY
        NAMES mpg123
        PATHS
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /usr/lib64
    )

    if (NOT MPG123_INCLUDE_DIR OR NOT MPG123_LIBRARY)
        message(FATAL_ERROR "mpg123 not found. Install libmpg123-dev (Debian/Ubuntu) or mpg123-devel / mpg123 for your distro.")
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDL3_INCLUDE_DIRS}
        ${SDL3_IMAGE_INCLUDE_DIRS}
        ${SDL3_TTF_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIR}
        ${MPG123_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        GLEW::GLEW
        assimp::assimp
	OpenAL::OpenAL
	fmt::fmt
        ${MPG123_LIBRARY}
    )
elseif(APPLE)
    message(STATUS "Configuring for macOS build")

    find_package(OpenGL REQUIRED)

    # SDL3 系 (Homebrew のパスを HINTS に追加)
    find_package(SDL3        REQUIRED CONFIG HINTS /opt/homebrew/opt/sdl3)
    find_package(SDL3_image  REQUIRED CONFIG HINTS /opt/homebrew/opt/sdl3_image)
    find_package(SDL3_ttf    REQUIRED CONFIG HINTS /opt/homebrew/opt/sdl3_ttf)

    find_package(GLEW   REQUIRED HINTS /opt/homebrew/opt/glew)
    find_package(ASSIMP REQUIRED HINTS /opt/homebrew/opt/assimp)
    find_package(OpenAL REQUIRED HINTS /opt/homebrew/opt/openal-soft)

    # ==== mpg123 を直接探す ====
    find_path(MPG123_INCLUDE_DIR
        NAMES mpg123.h
        HINTS
            /opt/homebrew/opt/mpg123/include
            /opt/homebrew/include
            /usr/local/include
    )

    find_library(MPG123_LIBRARY
        NAMES mpg123
        HINTS
            /opt/homebrew/opt/mpg123/lib
            /opt/homebrew/lib
            /usr/local/lib
    )

    if(NOT MPG123_INCLUDE_DIR OR NOT MPG123_LIBRARY)
        message(FATAL_ERROR "mpg123 not found. Please install with 'brew install mpg123'.")
    endif()

    # 必要なものだけ明示的に追加（SDL3 系は imported target に任せられる）
    target_include_directories(${PROJECT_NAME} PRIVATE
        /opt/homebrew/include
        ${GLEW_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIR}
        ${MPG123_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        GLEW::GLEW
        assimp::assimp
        ${OPENAL_LIBRARY}
        ${MPG123_LIBRARY}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path/../lib;/opt/homebrew/lib"
    )
endif()

#========================
# 資材コピー（共通）
#========================

set(SOURCE_SHADER_DIR       "${CMAKE_SOURCE_DIR}/${TOYLIB_PATH}/Shaders")
set(SOURCE_ASSET_DIR        "${CMAKE_SOURCE_DIR}/${GAME_PATH}/Assets")
set(SOURCE_SETTINGS_DIR     "${CMAKE_SOURCE_DIR}/${GAME_PATH}/Settings")
set(SOURCE_SYSSETTINGS_DIR  "${CMAKE_SOURCE_DIR}/${TOYLIB_PATH}/Settings")

# 実行ファイルのあるディレクトリを基準にコピー
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SOURCE_SHADER_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${TOYLIB_PATH}/Shaders"
    COMMENT "Copying Shaders to $<TARGET_FILE_DIR:${PROJECT_NAME}>/${TOYLIB_PATH}/Shaders"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SOURCE_ASSET_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${GAME_PATH}/Assets"
    COMMENT "Copying Assets to $<TARGET_FILE_DIR:${PROJECT_NAME}>/${GAME_PATH}/Assets"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SOURCE_SETTINGS_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${GAME_PATH}/Settings"
    COMMENT "Copying Settings to $<TARGET_FILE_DIR:${PROJECT_NAME}>/${GAME_PATH}/Settings"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SOURCE_SYSSETTINGS_DIR}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${TOYLIB_PATH}/Settings"
    COMMENT "Copying Settings to $<TARGET_FILE_DIR:${PROJECT_NAME}>/${TOYLIB_PATH}/Settings"
)
