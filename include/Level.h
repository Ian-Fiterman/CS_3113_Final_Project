#pragma once

#include "Entity.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "Target.h"

#include <vector>

// Simulation
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f; // physics step size in seconds
constexpr int PHYSICS_SUBSTEPS = 4;            // Box2D velocity/position iterations per step
constexpr float GRAVITY_Y = -10.0f;            // world gravity (m/s^2, Y-up)

// Hit sound
constexpr float HIT_EVENT_THRESHOLD = 2.0f; // minimum approach speed (m/s) to fire a hit event
constexpr int HIT_PITCH_MIN = 80;           // pitch randomisation range (percent)
constexpr int HIT_PITCH_MAX = 120;

// Mouse joint
constexpr float MOUSE_JOINT_HERTZ = 5.0f;
constexpr float MOUSE_JOINT_DAMPING = 0.7f;
constexpr float MOUSE_JOINT_FORCE_SCALE = 1000.0f; // multiplied by the grabbed body's mass

// Cursor atlas
constexpr int CURSOR_ATLAS_COLS = 3;
constexpr int CURSOR_ATLAS_ROWS = 1;
constexpr int CURSOR_RENDER_SIZE = 32; // drawn size in pixels
constexpr float CURSOR_HOTSPOT_X = 10.0f;
constexpr float CURSOR_HOTSPOT_Y = 6.0f;

// Win / fail status overlay
constexpr float STATUS_TEXT_DELAY = 2.0f; // seconds before the continue/retry prompt appears
constexpr int STATUS_FONT_SIZE = 50;
constexpr float STATUS_FONT_SPACING = 1.0f;
constexpr int STATUS_TEXT_Y = SCREEN_HEIGHT / 6 - 45;
constexpr int STATUS_LINE_SPACING = 50;

// Level label (top-centre scene identifier)
constexpr int LEVEL_LABEL_FONT_SIZE = 36;
constexpr float LEVEL_LABEL_FONT_SPACING = 1.0f;
constexpr float LEVEL_LABEL_Y = 20.0f;

// Target zone
constexpr float TARGET_FADE_DURATION = 2.0f; // seconds to fade target out after success
constexpr float TARGET_PADDING_PX = 10.0f;   // padding added around each level's target zone

// Shared level geometry defaults
constexpr float GROUND_HEIGHT_PX = 80.0f; // ground height used by every level
constexpr float GROUND_RECESS_PX = 3.0f;  // how far the target sinks into the ground body
constexpr float TEX_PADDING_PX = 50.0f;   // extra pixels drawn beyond the physics shape bounds

// Debug overlays
constexpr float DEBUG_NORMAL_LENGTH = 0.2f; // contact normal line length (m)
constexpr float DEBUG_CIRCLE_RADIUS = 4.0f; // contact point / joint marker radius (px)

enum CursorFrame { CURSOR_POINT = 0, CURSOR_OPEN = 1, CURSOR_CLOSED = 2 };

enum LevelStatus { NONE, SUCCESS, FAIL };

class Level : public Scene {
public:
    virtual ~Level();

protected:
    struct LevelState {
        // Physics world
        b2WorldId worldId = b2_nullWorldId;
        b2BodyId groundId = b2_nullBodyId;
        float groundWidthPx = 0.0f;
        float groundHeightPx = 0.0f;

        // Mouse joint drag
        b2JointId mouseJointId = b2_nullJointId;
        b2BodyId heldBodyId = b2_nullBodyId;
        float timeAccumulator = 0.0f;

        // Scene objects
        std::vector<Entity> entities;
        Target target;

        // Game state
        LevelStatus levelStatus = NONE;
        float elapsedSinceSuccess = 0.0f;
        bool successSoundPlayed = false;
    };

    LevelState mState;
    ResourceManager* mResources = nullptr; // non-owning, set in initialiseLevel()
    float mPreviousTicks = 0.0f;

    // Subclass interface
    virtual void spawnEntities() = 0;
    void resetLevel();

    // Entity spawn helpers - append to mState.entities using mState.worldId
    void spawnConvexAtCentroid(const std::vector<b2Vec2>& worldVerts, TextureInfo texInfo);
    void spawnCircleAt(float screenX, float screenY, float radiusPx, TextureInfo texInfo);

    // Setup
    void initialiseLevel(ResourceManager& resources, float groundWidthPx, float groundHeightPx);
    void createWorld();
    void createGround(float groundWidthPx, float groundHeightPx);

    // Scene overrides
    void processInput() override;
    void update(float dt) override;
    void render() override;

    // Physics / interaction
    void handleMouseJoint(Vector2 mousePos);
    void releaseMouseJoint();
    void pollHitEvents();
    static bool isEntityOffScreen(const Entity& entity);

    // Update helpers
    void tickSuccess(float dt);
    void stepPhysics(float dt);
    bool checkOffScreen();
    void checkWinCondition();

    // Rendering
    void renderBackground() const;
    void renderCursor() const;
    void renderTarget() const;
    void renderEntities() const;
    void renderLevelLabel() const;
    virtual void renderSuccessText() const;
    virtual void renderFailText() const;
    void renderDebug() const;

private:
    void createMouseJoint(b2Vec2 worldPos, Entity& hit);
    void updateMouseJoint(b2Vec2 target);
    void drawContactManifoldsDebug() const;
    void drawMouseJointDebug() const;
};
