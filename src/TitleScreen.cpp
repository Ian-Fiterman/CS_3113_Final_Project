#include "TitleScreen.h"

#include <string>

/**
 * @brief Initialises the title screen: physics world, text layout, and word render textures.
 */
void TitleScreen::initialise(ResourceManager& resources) {
    mCurrentSceneID = SCENE_TITLE;
    initialiseLevel(resources, TITLE_GROUND_WIDTH_PX, GROUND_HEIGHT_PX);

    const Vector2 neonSize = mResources->font.MeasureText("NEON", TITLE_FONT_SIZE, TITLE_FONT_SPACING);
    const Vector2 eonSize = mResources->font.MeasureText("EON", TITLE_FONT_SIZE, TITLE_FONT_SPACING);
    mNeonW = neonSize.x;
    mEonW = eonSize.x;
    mWordH = neonSize.y;

    const float totalW = mNeonW + TITLE_WORD_GAP + mEonW;
    mNeonX = (SCREEN_WIDTH - totalW) / 2.0f;
    mEonX = mNeonX + mNeonW + TITLE_WORD_GAP;
    mTitleY = SCREEN_HEIGHT * TITLE_Y_FRACTION;

    mNeonTex = raylib::RenderTexture(static_cast<int>(mNeonW + ITALIC_RIGHT_PAD_PX), static_cast<int>(mWordH));
    BeginTextureMode(mNeonTex);
    ClearBackground(BLANK);
    mResources->font.DrawText("NEON", {0.0f, 0.0f}, TITLE_FONT_SIZE, TITLE_FONT_SPACING, MAGENTA);
    EndTextureMode();

    mEonTex = raylib::RenderTexture(static_cast<int>(mEonW + ITALIC_RIGHT_PAD_PX), static_cast<int>(mWordH));
    BeginTextureMode(mEonTex);
    ClearBackground(BLANK);
    mResources->font.DrawText("EON", {0.0f, 0.0f}, TITLE_FONT_SIZE, TITLE_FONT_SPACING, RETRO_CYAN);
    EndTextureMode();
}

/**
 * @brief Handles input for both title phases.
 */
void TitleScreen::processInput() {
    if (IsKeyPressed(KEY_GRAVE)) mDebugMode = !mDebugMode;

    if (mTitlePhase == TITLE_IDLE) {
        if (IsKeyPressed(KEY_ENTER)) {
            mResources->enterSfx.Play();
            spawnEntities();
            buildTarget();
            mTitlePhase = TITLE_TUTORIAL;
        }
        return;
    }

    if (mState.levelStatus == FAIL) {
        if (IsKeyPressed(KEY_ENTER)) {
            mResources->enterSfx.Play();
            resetLevel();
        }
        return;
    }

    const bool canAdvance = mState.levelStatus == SUCCESS && mState.elapsedSinceSuccess > STATUS_TEXT_DELAY;
    if (IsKeyPressed(KEY_ENTER) && canAdvance) {
        mResources->enterSfx.Play();
        beginFadeOut(SCENE_LEVEL1);
        return;
    }

    if (mState.levelStatus == NONE) handleMouseJoint(GetMousePosition());
}

/**
 * @brief Steps physics, checks for off-screen bodies, and checks win condition.
 */
void TitleScreen::update(float dt) {
    if (!mResources) return;
    mResources->bgm.Update();

    if (mTitlePhase != TITLE_IDLE) tickSuccess(dt);
    if (mTitlePhase == TITLE_IDLE || mState.levelStatus != NONE) return;

    stepPhysics(dt);
    pollHitEvents();

    if (B2_IS_NON_NULL(mState.mouseJointId)) b2Body_SetAwake(mState.heldBodyId, true);

    if (checkOffScreen()) return;
    mState.target.update(mState.entities);
    checkWinCondition();
}

/**
 * @brief Renders the appropriate phase: static title or interactive tutorial.
 */
void TitleScreen::render() {
    renderBackground();

    if (mTitlePhase == TITLE_IDLE) {
        renderIdle();
        return;
    }

    DrawRectangle(0, SCREEN_HEIGHT - static_cast<int>(GROUND_HEIGHT_PX), SCREEN_WIDTH,
                  static_cast<int>(GROUND_HEIGHT_PX), DARKGRAY);

    renderTarget();
    renderEntities();
    renderTutorialText();

    if (mState.levelStatus == SUCCESS) renderSuccessText();
    if (mState.levelStatus == FAIL) renderFailText();

    if (mDebugMode) renderDebug();
    renderCursor();
}

/**
 * @brief Spawns two box bodies textured with the pre-baked NEON and EON render textures.
 *        Body width includes the italic right overhang so the collider matches the visible glyph bounds.
 *        Body center is shifted right by half the pad to keep the left edge flush with the text origin.
 */
void TitleScreen::spawnEntities() {
    const float bodyHalfH = (mWordH - AABB_TOP_TRIM - AABB_BOTTOM_TRIM) / 2.0f;
    const float centerY = mTitleY + mWordH / 2.0f - TEXT_BODY_OFFSET_PX;
    const float neonHalfW = (mNeonW + ITALIC_RIGHT_PAD_PX) / 2.0f;
    const float eonHalfW = (mEonW + ITALIC_RIGHT_PAD_PX) / 2.0f;
    const float neonCX = mNeonX + neonHalfW;
    const float eonCX = mEonX + eonHalfW;
    const b2Vec2 aabbOffset = {0.0f, -toMeters(TEXT_BODY_OFFSET_PX)};
    // NEON
    const TextureInfo neonInfo = {reinterpret_cast<const raylib::Texture*>(&mNeonTex.texture), 0.0f, false, true};
    mState.entities.push_back(Entity::createBox(mState.worldId, neonCX, centerY, neonHalfW, bodyHalfH, neonInfo));
    mState.entities.back().setLocalAABBOffset(aabbOffset);
    mState.entities.back().setLocalAABBExtents({mNeonW + ITALIC_RIGHT_PAD_PX, mWordH});
    // EON
    const TextureInfo eonInfo = {reinterpret_cast<const raylib::Texture*>(&mEonTex.texture), 0.0f, false, true};
    mState.entities.push_back(Entity::createBox(mState.worldId, eonCX, centerY, eonHalfW, bodyHalfH, eonInfo));
    mState.entities.back().setLocalAABBOffset(aabbOffset);
    mState.entities.back().setLocalAABBExtents({mEonW + ITALIC_RIGHT_PAD_PX, mWordH});
}

/**
 * @brief Builds the stacked T-shaped target sized to match the entity AABBs.
 *        EON slot sits at ground level; NEON slot stacks on top.
 *        Side padding on both sides; top padding only at the very top of the structure.
 */
void TitleScreen::buildTarget() {
    const float bodyHalfH = (mWordH - AABB_TOP_TRIM - AABB_BOTTOM_TRIM) / 2.0f;
    const float bodyH = 2.0f * bodyHalfH;
    const float neonHalfW_W = toMeters((mNeonW + ITALIC_RIGHT_PAD_PX) / 2.0f + TARGET_SIDE_PADDING_PX);
    const float eonHalfW_W = toMeters((mEonW + ITALIC_RIGHT_PAD_PX) / 2.0f + TARGET_SIDE_PADDING_PX);
    const float groundY = toMeters(GROUND_HEIGHT_PX - GROUND_RECESS_PX);
    const float centerXW = toMeters(SCREEN_WIDTH / 2.0f);
    const float eonMidY = groundY + toMeters(bodyH);
    const float neonTopY = eonMidY + toMeters(bodyH + TARGET_TOP_PADDING_PX);

    const std::vector<b2Vec2> eonPiece = {{centerXW - eonHalfW_W, groundY},
                                          {centerXW + eonHalfW_W, groundY},
                                          {centerXW + eonHalfW_W, eonMidY},
                                          {centerXW - eonHalfW_W, eonMidY}};

    const std::vector<b2Vec2> neonPiece = {{centerXW - neonHalfW_W, eonMidY},
                                           {centerXW + neonHalfW_W, eonMidY},
                                           {centerXW + neonHalfW_W, neonTopY},
                                           {centerXW - neonHalfW_W, neonTopY}};

    const std::vector<b2Vec2> outline = {{centerXW - eonHalfW_W, groundY},   {centerXW + eonHalfW_W, groundY},
                                         {centerXW + eonHalfW_W, eonMidY},   {centerXW + neonHalfW_W, eonMidY},
                                         {centerXW + neonHalfW_W, neonTopY}, {centerXW - neonHalfW_W, neonTopY},
                                         {centerXW - neonHalfW_W, eonMidY},  {centerXW - eonHalfW_W, eonMidY}};

    mState.target = Target(mState.worldId, outline, {eonPiece, neonPiece});
}

/**
 * @brief Renders the static title text and enter prompt.
 */
void TitleScreen::renderIdle() const {
    mResources->font.DrawText("NEON", {mNeonX, mTitleY}, TITLE_FONT_SIZE, TITLE_FONT_SPACING, MAGENTA);
    mResources->font.DrawText("EON", {mEonX, mTitleY}, TITLE_FONT_SIZE, TITLE_FONT_SPACING, RETRO_CYAN);

    const std::string prompt = "Press ENTER to Play";
    const float promptW = mResources->font.MeasureText(prompt, PROMPT_FONT_SIZE, PROMPT_SPACING).x;
    mResources->font.DrawText(prompt, {(SCREEN_WIDTH - promptW) / 2.0f, mTitleY + mWordH + IDLE_PROMPT_GAP_PX},
                              PROMPT_FONT_SIZE, PROMPT_SPACING, WHITE);
}

/** @brief Renders the two tutorial instruction lines. */
void TitleScreen::renderTutorialText() const {
    const std::string line1 = "Drag shapes with M1";
    const std::string line2 = "Fit shapes into the target to win";
    const float l1width = mResources->font.MeasureText(line1, PROMPT_FONT_SIZE, PROMPT_SPACING).x;
    const float l2width = mResources->font.MeasureText(line2, PROMPT_FONT_SIZE, PROMPT_SPACING).x;
    mResources->font.DrawText(line1, {(SCREEN_WIDTH - l1width) / 2.0f, STATUS_TEXT_Y}, PROMPT_FONT_SIZE, PROMPT_SPACING,
                              WHITE);
    mResources->font.DrawText(line2, {(SCREEN_WIDTH - l2width) / 2.0f, STATUS_TEXT_Y + TUTORIAL_LINE_SPACING},
                              PROMPT_FONT_SIZE, PROMPT_SPACING, WHITE);
}

/** @brief Draws the title-screen success prompt after the delay elapses. */
void TitleScreen::renderSuccessText() const {
    if (mState.elapsedSinceSuccess <= STATUS_TEXT_DELAY) return;
    const std::string begin = "Press ENTER to Begin";
    const float w = mResources->font.MeasureText(begin, PROMPT_FONT_SIZE, PROMPT_SPACING).x;
    mResources->font.DrawText(begin, {(SCREEN_WIDTH - w) / 2.0f, STATUS_TEXT_Y + 2.0f * TUTORIAL_LINE_SPACING},
                              PROMPT_FONT_SIZE, PROMPT_SPACING, GREEN);
}

/** @brief Draws the title-screen fail message and retry prompt. */
void TitleScreen::renderFailText() const {
    const std::string lost = "A shape fell off screen!";
    const std::string retry = "Press ENTER to try again";
    const float lw = mResources->font.MeasureText(lost, PROMPT_FONT_SIZE, PROMPT_SPACING).x;
    const float rw = mResources->font.MeasureText(retry, PROMPT_FONT_SIZE, PROMPT_SPACING).x;
    mResources->font.DrawText(lost, {(SCREEN_WIDTH - lw) / 2.0f, STATUS_TEXT_Y + 2.0f * TUTORIAL_LINE_SPACING},
                              PROMPT_FONT_SIZE, PROMPT_SPACING, RED);
    mResources->font.DrawText(retry, {(SCREEN_WIDTH - rw) / 2.0f, STATUS_TEXT_Y + 3.0f * TUTORIAL_LINE_SPACING},
                              PROMPT_FONT_SIZE, PROMPT_SPACING, RED);
}
