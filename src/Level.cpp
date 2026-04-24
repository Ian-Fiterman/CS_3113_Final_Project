#include "Level.h"

#include <string>

/** @brief Stores the resource reference, creates the physics world, and builds the ground body. */
void Level::initialiseLevel(ResourceManager& resources, float groundWidthPx, float groundHeightPx) {
    mResources = &resources;
    createWorld();
    createGround(groundWidthPx, groundHeightPx);
    HideCursor();
}

/** @brief Toggles debug mode with grave/tilde; ENTER advances to the next scene on success or resets on fail. */
void Level::processInput() {
    if (IsKeyPressed(KEY_GRAVE)) mDebugMode = !mDebugMode;

    if (IsKeyPressed(KEY_ENTER)) {
        const bool delayElapsed = mState.elapsedSinceSuccess > STATUS_TEXT_DELAY;
        if (mState.levelStatus == SUCCESS && delayElapsed) {
            mResources->enterSfx.Play();
            beginFadeOut(static_cast<SceneID>(static_cast<int>(mCurrentSceneID) + 1));
        } else if (mState.levelStatus == FAIL) {
            mResources->enterSfx.Play();
            resetLevel();
        }
    }

    if (mState.levelStatus != NONE) return;
    handleMouseJoint(GetMousePosition());
}

/** @brief Advances elapsedSinceSuccess and fires the success sound once after the delay. */
void Level::tickSuccess(float dt) {
    if (mState.levelStatus != SUCCESS) return;
    mState.elapsedSinceSuccess += dt;
    if (!mState.successSoundPlayed && mState.elapsedSinceSuccess > STATUS_TEXT_DELAY) {
        mResources->successSfx.Play();
        mState.successSoundPlayed = true;
    }
}

/** @brief Advances the fixed-timestep accumulator and steps the Box2D world. */
void Level::stepPhysics(float dt) {
    mState.timeAccumulator += dt;
    while (mState.timeAccumulator >= FIXED_TIMESTEP) {
        b2World_Step(mState.worldId, FIXED_TIMESTEP, PHYSICS_SUBSTEPS);
        mState.timeAccumulator -= FIXED_TIMESTEP;
    }
}

/** @brief Sets FAIL and plays the fail sound if any entity has left the screen. Returns true if failed. */
bool Level::checkOffScreen() {
    for (const auto& entity : mState.entities) {
        if (isEntityOffScreen(entity)) {
            releaseMouseJoint();
            mState.levelStatus = FAIL;
            mResources->failSfx.Play();
            return true;
        }
    }
    return false;
}

/** @brief Sets SUCCESS if all entities are inside the target. */
void Level::checkWinCondition() {
    if (mState.target.allInside() && mState.levelStatus != SUCCESS) {
        mState.levelStatus = SUCCESS;
        mState.elapsedSinceSuccess = 0.0f;
    }
}

/** @brief Steps the physics simulation, polls hit events, and checks for off-screen entities and the win condition. */
void Level::update(float dt) {
    mResources->bgm.Update();
    tickSuccess(dt);
    if (mState.levelStatus != NONE) return;

    stepPhysics(dt);
    pollHitEvents();

    // Keep held body awake so the mouse joint stays responsive.
    if (B2_IS_NON_NULL(mState.mouseJointId)) b2Body_SetAwake(mState.heldBodyId, true);

    mState.target.update(mState.entities);
    if (checkOffScreen()) return;
    checkWinCondition();
}

/** @brief Draws the background image scaled to fill the screen. */
void Level::renderBackground() const {
    const Rectangle bgSrc = {0.0f, 0.0f, static_cast<float>(mResources->background.width),
                             static_cast<float>(mResources->background.height)};
    mResources->background.Draw(bgSrc, {0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT}, {0.0f, 0.0f}, 0.0f, WHITE);
}

/** @brief Draws the background, ground, entities, target, status overlays, and cursor. */
void Level::render() {
    renderBackground();
    // Draw ground as a rectangle for dynamic sizing
    const float groundLeft = (SCREEN_WIDTH - mState.groundWidthPx) / 2.0f;
    DrawRectangle(static_cast<int>(groundLeft), SCREEN_HEIGHT - static_cast<int>(mState.groundHeightPx),
                  static_cast<int>(mState.groundWidthPx), static_cast<int>(mState.groundHeightPx), DARKGRAY);

    renderTarget();
    renderEntities();
    renderLevelLabel();

    if (mState.levelStatus == SUCCESS) renderSuccessText();
    if (mState.levelStatus == FAIL) renderFailText();
    if (mDebugMode) renderDebug();

    renderCursor();
}

/** @brief Draws contact manifolds, the mouse joint line, and the FPS counter. */
void Level::renderDebug() const {
    drawContactManifoldsDebug();
    if (B2_IS_NON_NULL(mState.mouseJointId)) drawMouseJointDebug();
    DrawFPS(SCREEN_WIDTH - 100, 10);
}

/** @brief Draws the level number label centred at the top of the screen. */
void Level::renderLevelLabel() const {
    const std::string label(TextFormat("LEVEL %d", static_cast<int>(mCurrentSceneID)));
    const float w = mResources->font.MeasureText(label, LEVEL_LABEL_FONT_SIZE, LEVEL_LABEL_FONT_SPACING).x;
    mResources->font.DrawText(label, {(SCREEN_WIDTH - w) / 2.0f, LEVEL_LABEL_Y}, LEVEL_LABEL_FONT_SIZE,
                              LEVEL_LABEL_FONT_SPACING, WHITE);
}

/** @brief Draws the LEVEL COMPLETE message and continue prompt after the status delay elapses. */
void Level::renderSuccessText() const {
    if (mState.elapsedSinceSuccess <= STATUS_TEXT_DELAY) return;

    const std::string line1 = "LEVEL COMPLETE!";
    const std::string line2 = "Press Enter to Continue";

    mResources->font.DrawText(
        line1,
        {(SCREEN_WIDTH - mResources->font.MeasureText(line1, STATUS_FONT_SIZE, STATUS_FONT_SPACING).x) / 2.0f,
         static_cast<float>(STATUS_TEXT_Y)},
        STATUS_FONT_SIZE, STATUS_FONT_SPACING, GREEN);

    mResources->font.DrawText(
        line2,
        {(SCREEN_WIDTH - mResources->font.MeasureText(line2, STATUS_FONT_SIZE, STATUS_FONT_SPACING).x) / 2.0f,
         static_cast<float>(STATUS_TEXT_Y + STATUS_LINE_SPACING)},
        STATUS_FONT_SIZE, STATUS_FONT_SPACING, GREEN);
}

/** @brief Draws the LEVEL FAILED message and retry prompt. */
void Level::renderFailText() const {
    const std::string line1 = "LEVEL FAILED!";
    const std::string line2 = "Press Enter to Retry";

    mResources->font.DrawText(
        line1,
        {(SCREEN_WIDTH - mResources->font.MeasureText(line1, STATUS_FONT_SIZE, STATUS_FONT_SPACING).x) / 2.0f,
         static_cast<float>(STATUS_TEXT_Y)},
        STATUS_FONT_SIZE, STATUS_FONT_SPACING, RED);

    mResources->font.DrawText(
        line2,
        {(SCREEN_WIDTH - mResources->font.MeasureText(line2, STATUS_FONT_SIZE, STATUS_FONT_SPACING).x) / 2.0f,
         static_cast<float>(STATUS_TEXT_Y + STATUS_LINE_SPACING)},
        STATUS_FONT_SIZE, STATUS_FONT_SPACING, RED);
}

/** @brief Renders the target zone, fading it out over TARGET_FADE_DURATION seconds after success. */
void Level::renderTarget() const {
    if (mState.levelStatus == SUCCESS) {
        const float alpha = 1.0f - Clamp(mState.elapsedSinceSuccess / TARGET_FADE_DURATION, 0.0f, 1.0f);
        mState.target.render({0, 228, 48, static_cast<unsigned char>(alpha * 255)});
    } else {
        mState.target.render(RED);
    }
}

/** @brief Renders all entities; applies the spectrum shader on success, and invert shader to the held entity. */
void Level::renderEntities() const {
    SetShaderValue(mResources->spectrumShader, mResources->spectrumTimeLoc, &mState.elapsedSinceSuccess,
                   SHADER_UNIFORM_FLOAT);
    for (const auto& entity : mState.entities) {
        const bool held = B2_ID_EQUALS(entity.bodyId(), mState.heldBodyId);
        if (mState.levelStatus == SUCCESS)
            entity.render(BLACK, mDebugMode, &mResources->spectrumShader);
        else
            entity.render(BLACK, mDebugMode, held ? &mResources->invertShader : nullptr);
    }
}

/** @brief Computes the centroid of worldVerts in screen space and spawns a convex body there. */
void Level::spawnConvexAtCentroid(const std::vector<b2Vec2>& worldVerts, TextureInfo texInfo) {
    const Vector2 pos = toScreen(centroidOf(worldVerts));
    mState.entities.push_back(Entity::createConvexPolygon(mState.worldId, pos.x, pos.y, worldVerts, texInfo));
}

/** @brief Spawns a circle body at the given screen-space position with the given radius. */
void Level::spawnCircleAt(float screenX, float screenY, float radiusPx, TextureInfo texInfo) {
    mState.entities.push_back(Entity::createCircle(mState.worldId, screenX, screenY, radiusPx, texInfo));
}

/** @brief Destroys all entity bodies, resets game state, and re-calls spawnEntities(). */
void Level::resetLevel() {
    releaseMouseJoint();
    for (const auto& entity : mState.entities)
        b2DestroyBody(entity.bodyId());
    mState.entities.clear();
    mState.levelStatus = NONE;
    mState.elapsedSinceSuccess = 0.0f;
    mState.timeAccumulator = 0.0f;
    mState.successSoundPlayed = false;
    spawnEntities();
}

/** @brief Releases the mouse joint, destroys the Box2D world, and restores the system cursor. */
Level::~Level() {
    if (!b2World_IsValid(mState.worldId)) return;
    ShowCursor();
    releaseMouseJoint();
    b2DestroyWorld(mState.worldId);
    mState.entities.clear();
}

/** @brief Initialises the Box2D world with gravity and resets all game state fields. */
void Level::createWorld() {
    mNextSceneID = SCENE_NONE;
    mState.levelStatus = NONE;
    mState.elapsedSinceSuccess = 0.0f;
    mState.timeAccumulator = 0.0f;
    mState.successSoundPlayed = false;

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, GRAVITY_Y};
    worldDef.hitEventThreshold = HIT_EVENT_THRESHOLD;
    mState.worldId = b2CreateWorld(&worldDef);
}

/** @brief Creates a static ground body centred on screen, sized to groundWidthPx by groundHeightPx. */
void Level::createGround(float groundWidthPx, float groundHeightPx) {
    mState.groundWidthPx = groundWidthPx;
    mState.groundHeightPx = groundHeightPx;

    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = {toMeters(SCREEN_WIDTH / 2.0f), toMeters(groundHeightPx / 2.0f)};
    mState.groundId = b2CreateBody(mState.worldId, &groundDef);

    b2Polygon groundBox = b2MakeBox(toMeters(groundWidthPx / 2.0f), toMeters(groundHeightPx / 2.0f));
    b2ShapeDef groundShape = b2DefaultShapeDef();
    b2CreatePolygonShape(mState.groundId, &groundShape, &groundBox);
}

/** @brief On left-mouse press, finds the entity under the cursor and creates a mouse joint.
 *         Moves the joint target while the button is held, then destroys the joint on release. */
void Level::handleMouseJoint(Vector2 mousePos) {
    const b2Vec2 worldPos = toWorld(mousePos);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Entity* hit = entityAtWorldPoint(mState.entities, worldPos);
        if (hit) createMouseJoint(worldPos, *hit);
    }

    if (B2_IS_NON_NULL(mState.mouseJointId) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) updateMouseJoint(worldPos);

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && B2_IS_NON_NULL(mState.mouseJointId)) releaseMouseJoint();
}

/** @brief Creates a Box2D mouse joint anchored to the ground body and targeting the hit entity. */
void Level::createMouseJoint(b2Vec2 worldPos, Entity& hit) {
    b2MouseJointDef mjd = b2DefaultMouseJointDef();
    mjd.bodyIdA = mState.groundId;
    mjd.bodyIdB = hit.bodyId();
    mjd.target = worldPos;
    mjd.hertz = MOUSE_JOINT_HERTZ;
    mjd.dampingRatio = MOUSE_JOINT_DAMPING;
    mjd.maxForce = MOUSE_JOINT_FORCE_SCALE * b2Body_GetMass(hit.bodyId());
    mjd.collideConnected = true;
    mState.mouseJointId = b2CreateMouseJoint(mState.worldId, &mjd);
    mState.heldBodyId = hit.bodyId();
    b2Body_SetAwake(hit.bodyId(), true);

    if (mResources) mResources->grabSfx.Play();
}

/** @brief Moves the mouse joint target, clamping to screen bounds and snapping above the ground surface. */
void Level::updateMouseJoint(b2Vec2 target) {
    // Clamp target to screen bounds
    target.x = Clamp(target.x, 0.0f, toMeters(SCREEN_WIDTH));
    target.y = Clamp(target.y, 0.0f, toMeters(SCREEN_HEIGHT));
    // Compute ground surface and edges in world space for clamping and snapping
    const float groundSurface = toMeters(mState.groundHeightPx);
    const float groundLeft = toMeters((SCREEN_WIDTH - mState.groundWidthPx) / 2.0f);
    const float groundRight = toMeters((SCREEN_WIDTH + mState.groundWidthPx) / 2.0f);

    const bool overGround = target.x >= groundLeft && target.x <= groundRight;
    const bool atGroundLevel = target.y < groundSurface;

    if (overGround && atGroundLevel) {
        target.y = groundSurface; // snap to surface directly above the ground
    } else if (!overGround && atGroundLevel) {
        if (target.x < groundLeft) target.x = Clamp(target.x, 0.0f, groundLeft);
        if (target.x > groundRight) target.x = Clamp(target.x, groundRight, toMeters(SCREEN_WIDTH));
    }

    b2MouseJoint_SetTarget(mState.mouseJointId, target);
}

/** @brief Destroys the active mouse joint and clears the held body reference. */
void Level::releaseMouseJoint() {
    if (B2_IS_NON_NULL(mState.mouseJointId)) {
        b2DestroyJoint(mState.mouseJointId);
        mState.mouseJointId = b2_nullJointId;
        mState.heldBodyId = b2_nullBodyId;
    }
}

/** @brief Plays the hit sound for the first impact event this frame that exceeds the approach-speed threshold. */
void Level::pollHitEvents() {
    if (!mResources) return;
    const b2ContactEvents events = b2World_GetContactEvents(mState.worldId);
    for (int i = 0; i < events.hitCount; i++) {
        if (!mResources->hitSfx.IsPlaying()) {
            const float pitch = static_cast<float>(GetRandomValue(HIT_PITCH_MIN, HIT_PITCH_MAX)) / 100.0f;
            mResources->hitSfx.SetPitch(pitch);
            mResources->hitSfx.Play();
        }
        return;
    }
}

/** @brief Returns true if the entity's oriented bounding box has fully left the visible screen area. */
bool Level::isEntityOffScreen(const Entity& entity) {
    const Vector2 center = toScreen(b2Body_GetPosition(entity.bodyId()));
    const b2Vec2 extents = entity.localAABBExtents();
    const b2Rot rot = b2Body_GetRotation(entity.bodyId());
    const float halfW = extents.x / 2.0f;
    const float halfH = extents.y / 2.0f;
    // Project OBB axes onto screen X/Y to get world-aligned half-extents.
    const float wHalfW = fabsf(halfW * rot.c) + fabsf(halfH * rot.s);
    const float wHalfH = fabsf(halfW * rot.s) + fabsf(halfH * rot.c);
    // No top bound check due to gravity
    return center.x + wHalfW < 0.0f || center.x - wHalfW > SCREEN_WIDTH || center.y - wHalfH > SCREEN_HEIGHT;
}

/** @brief Draws the cursor sprite from the atlas, switching frame based on hover or grab state. */
void Level::renderCursor() const {
    if (!mResources) return;

    const Vector2 mousePos = GetMousePosition();

    int frameIndex;
    if (B2_IS_NON_NULL(mState.mouseJointId)) {
        frameIndex = CURSOR_CLOSED;
    } else {
        const b2Vec2 worldPos = toWorld(mousePos);
        bool hovering = false;
        for (const auto& entity : mState.entities) {
            if (b2Shape_TestPoint(entity.shapeId(), worldPos)) {
                hovering = true;
                break;
            }
        }
        frameIndex = hovering ? CURSOR_OPEN : CURSOR_POINT;
    }

    const float frameW = static_cast<float>(mResources->cursorAtlas.width) / CURSOR_ATLAS_COLS;
    const float frameH = static_cast<float>(mResources->cursorAtlas.height) / CURSOR_ATLAS_ROWS;

    const Rectangle src = {frameIndex * frameW, 0.0f, frameW, frameH};
    const Rectangle dest = {mousePos.x - CURSOR_HOTSPOT_X, mousePos.y - CURSOR_HOTSPOT_Y,
                            static_cast<float>(CURSOR_RENDER_SIZE), static_cast<float>(CURSOR_RENDER_SIZE)};
    mResources->cursorAtlas.Draw(src, dest, {0.0f, 0.0f}, 0.0f, WHITE);
}

/** @brief Draws contact points (purple) and outward normals (orange) for each entity in contact. */
void Level::drawContactManifoldsDebug() const {
    for (const auto& entity : mState.entities) {
        const int capacity = b2Body_GetContactCapacity(entity.bodyId());
        if (capacity == 0) continue;

        std::vector<b2ContactData> contacts(capacity);
        const int count = b2Body_GetContactData(entity.bodyId(), contacts.data(), capacity);

        for (int i = 0; i < count; i++) {
            const b2Manifold& m = contacts[i].manifold;
            for (int j = 0; j < m.pointCount; j++) {
                if (m.points[j].separation > 0.0f) continue;
                const Vector2 point = toScreen(m.points[j].point);
                const Vector2 normalEnd = toScreen({m.points[j].point.x + m.normal.x * DEBUG_NORMAL_LENGTH,
                                                    m.points[j].point.y + m.normal.y * DEBUG_NORMAL_LENGTH});
                DrawCircleV(point, DEBUG_CIRCLE_RADIUS, PURPLE);
                DrawLineV(point, normalEnd, ORANGE);
            }
        }
    }
}

/** @brief Draws a line from the held body centre to the mouse joint target, with coloured endpoint markers. */
void Level::drawMouseJointDebug() const {
    if (B2_IS_NULL(mState.mouseJointId)) return;

    const b2Vec2 bodyPos = b2Body_GetPosition(mState.heldBodyId);
    const b2Vec2 target = b2MouseJoint_GetTarget(mState.mouseJointId);
    const Vector2 bodyScreen = toScreen(bodyPos);
    const Vector2 targetScreen = toScreen(target);

    DrawLineV(bodyScreen, targetScreen, BLACK);
    DrawCircleV(targetScreen, DEBUG_CIRCLE_RADIUS, GREEN);
    DrawCircleV(bodyScreen, DEBUG_CIRCLE_RADIUS, YELLOW);
}
