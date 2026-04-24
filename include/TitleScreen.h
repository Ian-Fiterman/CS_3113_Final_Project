#pragma once
#include "Level.h"

enum TitlePhase { TITLE_IDLE, TITLE_TUTORIAL };

constexpr Color RETRO_CYAN = {0, 240, 255, 255};

constexpr int TITLE_FONT_SIZE = 120;
constexpr float TITLE_FONT_SPACING = 2.0f;
constexpr int PROMPT_FONT_SIZE = 36;
constexpr float PROMPT_SPACING = 1.0f;

constexpr float TITLE_WORD_GAP = 30.0f;
constexpr float AABB_TOP_TRIM = 7.0f;
constexpr float AABB_BOTTOM_TRIM = 33.0f;
constexpr float TEXT_BODY_OFFSET_PX = 13.0f;
constexpr float TARGET_TOP_PADDING_PX = 10.0f;
constexpr float TARGET_SIDE_PADDING_PX = 10.0f;
constexpr float ITALIC_RIGHT_PAD_PX = 25.0f;

// Physics
constexpr float TITLE_GROUND_WIDTH_PX = static_cast<float>(SCREEN_WIDTH);

// Layout
constexpr float TITLE_Y_FRACTION = 0.35f;   // fraction of screen height for title text baseline
constexpr float IDLE_PROMPT_GAP_PX = 24.0f; // gap between title text and "Press ENTER" prompt

constexpr float TUTORIAL_LINE_SPACING = 44.0f;

class TitleScreen : public Level {
public:
    void initialise(ResourceManager& resources) override;

private:
    TitlePhase mTitlePhase = TITLE_IDLE;

    float mNeonX = 0.0f;
    float mEonX = 0.0f;
    float mTitleY = 0.0f;
    float mNeonW = 0.0f;
    float mEonW = 0.0f;
    float mWordH = 0.0f;

    raylib::RenderTexture mNeonTex;
    raylib::RenderTexture mEonTex;

    void processInput() override;
    void update(float dt) override;
    void render() override;

    void spawnEntities() override;
    void buildTarget();
    void renderIdle() const;
    void renderTutorialText() const;
    void renderSuccessText() const override;
    void renderFailText() const override;
};
