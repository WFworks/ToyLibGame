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

    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(GLEW CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(assimp CONFIG REQUIRED)
    # openal-soft / mpg123 (vcpkg 想定)
    find_package(OpenAL CONFIG REQUIRED)

    target_link_libraries(${PROJECT_NAME}
        OpenGL32
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        GLEW::GLEW
        assimp::assimp
        OpenAL::OpenAL
        mpg123          # vcpkg でインストールしてあれば見つかるはず
    )

    # SDLのエントリポイント無効化
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

    # MSVC専用設定（Visual Studio系コンパイラ）
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /MP)
        target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:CONSOLE)
    endif()

    # vcpkg の DLL 置き場（自分の環境に合わせてここだけ調整）
    set(VCPKG_BIN_DIR "C:/tools/vcpkg/installed/x64-windows/bin")

    set(DLL_FILES
        "${VCPKG_BIN_DIR}/SDL2.dll"
        "${VCPKG_BIN_DIR}/SDL2_image.dll"
        "${VCPKG_BIN_DIR}/jpeg62.dll"
        "${VCPKG_BIN_DIR}/libpng16.dll"
        "${VCPKG_BIN_DIR}/zlib1.dll"
        "${VCPKG_BIN_DIR}/SDL2_ttf.dll"
        "${VCPKG_BIN_DIR}/glew32.dll"
        "${VCPKG_BIN_DIR}/assimp-vc143-mt.dll"

        # OpenAL-soft 本体
        "${VCPKG_BIN_DIR}/openal32.dll"

        # BGM デコード用 mpg123
        "${VCPKG_BIN_DIR}/mpg123.dll"
    )

    # 実行ファイルと同じディレクトリに DLL をコピー
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
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(MPG123 REQUIRED mpg123)
    find_package(OpenAL REQUIRED)         # openal-soft が裏で使われることが多い
    find_package(GLEW REQUIRED)
    pkg_check_modules(ASSIMP REQUIRED assimp)

    include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${MPG123_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${MPG123_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${ASSIMP_LIBRARIES}
        OpenAL::OpenAL
    )

elseif(APPLE)
    message(STATUS "Configuring for macOS build")

    find_package(OpenGL REQUIRED)

    find_package(SDL2 REQUIRED HINTS /opt/homebrew/opt/sdl2)
    find_package(SDL2_image REQUIRED HINTS /opt/homebrew/opt/sdl2_image)
    find_package(SDL2_ttf REQUIRED HINTS /opt/homebrew/opt/sdl2_ttf)
    find_package(GLEW REQUIRED HINTS /opt/homebrew/opt/glew)
    find_package(ASSIMP REQUIRED HINTS /opt/homebrew/opt/assimp)
    find_package(OpenAL REQUIRED HINTS /opt/homebrew/opt/openal-soft)

    # ==== mpg123 を pkg-config じゃなくて直接探す ====
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

    target_include_directories(${PROJECT_NAME} PRIVATE
        /opt/homebrew/include
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIR}
        ${MPG123_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
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