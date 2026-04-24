cmake_minimum_required(VERSION 3.20)
project(neon_eon)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    raylib_cpp
    GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
    GIT_TAG        v5.5.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib_cpp)

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG        v3.1.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(box2d)

add_executable(neon_eon
    main.cpp
    src/Entity.cpp
    src/Target.cpp
    src/Level.cpp
    src/SceneManager.cpp
    src/Level1.cpp
    src/Level2.cpp
    src/Level3.cpp
    src/Level4.cpp
    src/TitleScreen.cpp
    src/EndScreen.cpp
)

target_include_directories(neon_eon PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${raylib_SOURCE_DIR}/src
    ${raylib_cpp_SOURCE_DIR}/include
    ${box2d_SOURCE_DIR}/include
)

target_link_libraries(neon_eon PRIVATE raylib raylib_cpp box2d)

# Copy assets folder next to the executable after every build
add_custom_command(TARGET neon_eon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:neon_eon>/assets
)

# use lld linker to avoid MinGW ld crashing on large link jobs
target_link_options(neon_eon PRIVATE -fuse-ld=lld)
