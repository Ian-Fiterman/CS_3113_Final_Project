#pragma once

#include "ResourceManager.h"

constexpr int SCREEN_WIDTH = 1600;
constexpr int SCREEN_HEIGHT = 1200;

enum SceneID {
    SCENE_NONE = -1,
    SCENE_TITLE = 0,
    SCENE_LEVEL1 = 1,
    SCENE_LEVEL2 = 2,
    SCENE_LEVEL3 = 3,
    SCENE_LEVEL4 = 4,
    SCENE_END = 5
};

enum Phase { ACTIVE = 0, FADING_OUT = 1, FADING_IN = 2 };

class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    virtual void initialise(ResourceManager& resources) = 0;
    virtual void processInput() = 0;
    virtual void update(float dt) = 0;
    virtual void render() = 0;

    SceneID nextScene() const { return mNextSceneID; }

    SceneID currentScene() const { return mCurrentSceneID; }

    Phase phase() const { return mPhase; }

    bool transitionComplete() const { return mTransitionTime >= mTransitionDuration; }

    void clearNextSceneID() { mNextSceneID = SCENE_NONE; }

    void beginFadeIn() {
        mPhase = FADING_IN;
        mTransitionTime = 0.0f;
    }

    void becomeActive() {
        mPhase = ACTIVE;
        mTransitionTime = 0.0f;
    }

    void beginFadeOut(SceneID next) {
        mNextSceneID = next;
        mPhase = FADING_OUT;
        mTransitionTime = 0.0f;
    }

    void advanceTransition(float dt) { mTransitionTime += dt; }

    bool debugMode() const { return mDebugMode; }

    virtual void fadeIn(float dt) {
        render();
        const float t = Clamp(mTransitionTime / mTransitionDuration, 0.0f, 1.0f);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, static_cast<unsigned char>((1.0f - t) * 255)});
    }

    virtual void fadeOut(float dt) {
        render();
        const float t = Clamp(mTransitionTime / mTransitionDuration, 0.0f, 1.0f);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0, 0, 0, static_cast<unsigned char>(t * 255)});
    }

protected:
    bool mDebugMode = false;
    SceneID mNextSceneID = SCENE_NONE;
    SceneID mCurrentSceneID = SCENE_NONE;
    Phase mPhase = ACTIVE;
    float mTransitionTime = 0.0f;
    float mTransitionDuration = 1.0f;
};
