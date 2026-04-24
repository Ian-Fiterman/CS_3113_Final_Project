#include "EndScreen.h"

/** @brief Stores the resource reference and sets the scene ID. */
void EndScreen::initialise(ResourceManager& resources) {
    mCurrentSceneID = SCENE_END;
    mResources = &resources;
}

/** @brief Begins a fade-out to the title screen when ENTER is pressed. */
void EndScreen::processInput() {
    if (IsKeyPressed(KEY_ENTER)) {
        mResources->enterSfx.Play();
        beginFadeOut(SCENE_TITLE);
    }
}

/** @brief Updates the background music stream. */
void EndScreen::update(float dt) {
    if (mResources) mResources->bgm.Update();
}

/** @brief Measures text width, then draws it centred horizontally at y with the given font parameters. */
void EndScreen::drawCentered(const std::string& text, float y, int fontSize, float spacing, Color color) const {
    const float w = mResources->font.MeasureText(text, fontSize, spacing).x;
    mResources->font.DrawText(text, {(SCREEN_WIDTH - w) / 2.0f, y}, fontSize, spacing, color);
}

/** @brief Draws the background, title, credits list, and return-to-title prompt. */
void EndScreen::render() {
    const Rectangle src = {0.0f, 0.0f, static_cast<float>(mResources->background.width),
                           static_cast<float>(mResources->background.height)};
    const Rectangle dest = {0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT};
    mResources->background.Draw(src, dest, {0.0f, 0.0f}, 0.0f, WHITE);

    drawCentered("Thanks for Playing", END_TITLE_Y, END_TITLE_FONT_SIZE, END_TITLE_FONT_SPACING, WHITE);
    drawCentered("Credits:", END_HEADER_Y, END_HEADER_FONT_SIZE, END_HEADER_FONT_SPACING, WHITE);

    for (int i = 0; i < static_cast<int>(END_CREDITS.size()); i++) {
        if (END_CREDITS[i].empty()) continue;
        drawCentered(END_CREDITS[i], END_CREDITS_START_Y + i * END_CREDIT_SPACING, END_CREDIT_FONT_SIZE,
                     END_CREDIT_FONT_SPACING, LIGHTGRAY);
    }

    drawCentered("Press ENTER to return to title screen", END_PROMPT_Y, END_PROMPT_FONT_SIZE, END_PROMPT_FONT_SPACING,
                 GRAY);
}
