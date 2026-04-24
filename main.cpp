/**
 * Author: Ian Fiterman
 * Assignment: Neon Eon
 * Date due: 4/24/26, 2:00pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "EndScreen.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "TitleScreen.h"

#include "raylib-cpp.hpp"

constexpr int FPS = 60;

int main() {
    // Window and audio device managed via RAII - CloseWindow() and
    // CloseAudioDevice() are called automatically on destruction.
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    raylib::Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Neon Eon");
    raylib::AudioDevice audio;
    window.SetTargetFPS(FPS);

    // ResourceManager and SceneManager live here by value.
    ResourceManager resourceManager;
    SceneManager sceneManager(resourceManager);

    sceneManager.registerScene<Level1>(SCENE_LEVEL1);
    sceneManager.registerScene<Level2>(SCENE_LEVEL2);
    sceneManager.registerScene<Level3>(SCENE_LEVEL3);
    sceneManager.registerScene<Level4>(SCENE_LEVEL4);
    sceneManager.registerScene<EndScreen>(SCENE_END);
    sceneManager.registerScene<TitleScreen>(SCENE_TITLE);
    sceneManager.switchTo(SCENE_TITLE);

    float previousTicks = static_cast<float>(GetTime());

    while (!window.ShouldClose() && !IsKeyPressed(KEY_Q)) {
        float ticks = static_cast<float>(GetTime());
        float dt = ticks - previousTicks;
        previousTicks = ticks;

        sceneManager.processInput();
        sceneManager.update(dt);
        sceneManager.render(dt);
    }

    return 0;
}