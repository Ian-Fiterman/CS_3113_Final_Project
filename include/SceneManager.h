#pragma once

#include "ResourceManager.h"
#include "Scene.h"

#include <concepts>
#include <functional>
#include <memory>
#include <unordered_map>

// Concept verifies that only types derived from Scene can be registered in the SceneManager.
template <typename T>
concept DerivedFromScene = std::derived_from<T, Scene>;

// I love concepts so much

class SceneManager {
public:
    // This is the most Java-like C++ code I've ever written LOL
    using SceneFactory = std::function<std::unique_ptr<Scene>()>;

    explicit SceneManager(ResourceManager& resources);
    ~SceneManager();

    template <DerivedFromScene T>
    void registerScene(SceneID id) {
        mFactories[id] = [] { return std::make_unique<T>(); };
    }

    void switchTo(SceneID id);

    void processInput();
    void update(float dt);
    void render(float dt);

    bool hasActive() const { return mActive != nullptr; }

private:
    void processDebugInput();

    ResourceManager& mResources;
    std::unordered_map<SceneID, SceneFactory> mFactories;
    std::unique_ptr<Scene> mActive;
    SceneID mActiveID = SCENE_NONE;
};
