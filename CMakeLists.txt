cmake_minimum_required(VERSION 3.20)
project(neon_eon)  # <-- only change this

# ---------------------------------------------------------------
# Expected directory layout:
#   my_project/
#   ├── CMakeLists.txt
#   ├── main.cpp
#   ├── include/        <- your header files (.h / .hpp)
#   ├── src/            <- your source files (.cpp)
#   │   ├── MyClass.cpp
#   │   └── ...
#   └── assets/         <- runtime assets (textures, sounds, etc.)
#
# What to change for a new project:
#   1. project(...) above       -> your project name (one place)
#   2. Add/remove .cpp files in add_executable if not using src/
#
# Build from project root:
#   cmake -B build && cmake --build build
#
# Run:
#   Windows:      ./build/neon_eon.exe
#   Linux/macOS:  ./build/neon_eon
# ---------------------------------------------------------------

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

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} main.cpp ${MY_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${raylib_SOURCE_DIR}/src
    ${raylib_cpp_SOURCE_DIR}/include
    ${box2d_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib raylib_cpp box2d)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,-subsystem,console -fuse-ld=lld)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m dl pthread)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
endif()
