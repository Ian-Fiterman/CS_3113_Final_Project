#include "SceneManager.h"

#include <stdexcept>

/**
 * @brief Constructs the SceneManager with a reference to the shared resource manager.
 * @param resources Shared resource manager passed to each scene on initialisation.
 */
SceneManager::SceneManager(ResourceManager& resources) : mResources(resources) { }

/** @brief Destructor. Resets the active scene, triggering its RAII cleanup. */
SceneManager::~SceneManager() {
    mActive.reset();
}

/**
 * @brief Destroys the current scene and constructs the one registered under id.
 *        Initialises the new scene and immediately begins its fade-in transition.
 * @param id SceneID of the scene to switch to.
 * @throws std::runtime_error if no scene is registered under id.
 */
void SceneManager::switchTo(SceneID id) {
    if (mActive) mActive.reset();

    const auto it = mFactories.find(id);
    if (it == mFactories.end())
        throw std::runtime_error("SceneManager: no scene registered with id " + std::to_string(id));

    mActive = it->second();
    mActiveID = id;
    mActive->initialise(mResources);
    mActive->beginFadeIn();
}

/**
 * @brief Forwards input to the active scene only when it is fully active.
 *        Input is suppressed during fade-in and fade-out transitions.
 */
void SceneManager::processInput() {
    if (!mActive || mActive->phase() != ACTIVE) return;
    processDebugInput();
    mActive->processInput();
}

/** @brief Allows debug-mode number-key shortcuts (0-5) to jump directly to any registered scene. */
void SceneManager::processDebugInput() {
    if (!mActive->debugMode()) return;
    if (IsKeyPressed(KEY_ZERO)) mActive->beginFadeOut(SCENE_TITLE);
    if (IsKeyPressed(KEY_ONE)) mActive->beginFadeOut(SCENE_LEVEL1);
    if (IsKeyPressed(KEY_TWO)) mActive->beginFadeOut(SCENE_LEVEL2);
    if (IsKeyPressed(KEY_THREE)) mActive->beginFadeOut(SCENE_LEVEL3);
    if (IsKeyPressed(KEY_FOUR)) mActive->beginFadeOut(SCENE_LEVEL4);
    if (IsKeyPressed(KEY_FIVE)) mActive->beginFadeOut(SCENE_END);
}

/**
 * @brief Updates the active scene and drives the transition state machine.
 *        Advances the transition timer during fades and switches scenes when
 *        a fade-out completes.
 * @param dt Frame delta time in seconds.
 */
void SceneManager::update(float dt) {
    if (!mActive) return;

    mActive->update(dt);

    if (mActive->phase() != ACTIVE) mActive->advanceTransition(dt);

    if (mActive->transitionComplete()) {
        switch (mActive->phase()) {
        case FADING_IN :
            mActive->becomeActive();
            break;
        case FADING_OUT :
            switchTo(mActive->nextScene()); // destroys current scene, constructs next
            break;
        case ACTIVE :
            break;
        }
    }
}

/**
 * @brief Renders the active scene, dispatching to the appropriate fade or normal render path.
 * @param dt Frame delta time in seconds, forwarded to fade render methods.
 */
void SceneManager::render(float dt) {
    if (!mActive) return;
    BeginDrawing();
    switch (mActive->phase()) {
    case ACTIVE :
        mActive->render();
        break;
    case FADING_IN :
        mActive->fadeIn(dt);
        break;
    case FADING_OUT :
        mActive->fadeOut(dt);
        break;
    }
    EndDrawing();
}
