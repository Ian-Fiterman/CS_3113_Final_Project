#pragma once

#include "Level.h" // for screen/font constants
#include "Scene.h"
#include <array>
#include <string>

constexpr int   END_TITLE_FONT_SIZE    = 80;
constexpr float END_TITLE_FONT_SPACING = 2.0f;
constexpr int   END_HEADER_FONT_SIZE   = 50;
constexpr float END_HEADER_FONT_SPACING = 1.0f;
constexpr int   END_CREDIT_FONT_SIZE   = 36;
constexpr float END_CREDIT_FONT_SPACING = 1.0f;
constexpr int   END_PROMPT_FONT_SIZE   = 28;
constexpr float END_PROMPT_FONT_SPACING = 1.0f;

constexpr float END_TITLE_Y        = 120.0f;
constexpr float END_HEADER_Y       = 280.0f;
constexpr float END_CREDITS_START_Y = 360.0f;
constexpr float END_CREDIT_SPACING = 56.0f;
constexpr float END_PROMPT_Y       = SCREEN_HEIGHT - 100.0f;

inline const std::string END_CREDIT_1 = "Developed by: Ian Fiterman";
inline const std::string END_CREDIT_2 = "Art by: Marisa Triola";
inline const std::string END_CREDIT_3 = "Special Thanks to: Prof. Romero Cruz";

inline const std::array<std::string, 3> END_CREDITS = {END_CREDIT_1, END_CREDIT_2, END_CREDIT_3};

class EndScreen : public Scene {
public:
    EndScreen() = default;
    ~EndScreen() = default;

    void initialise(ResourceManager& resources) override;
    void processInput() override;
    void update(float dt) override;
    void render() override;

private:
    ResourceManager* mResources = nullptr;

    void drawCentered(const std::string& text, float y, int fontSize, float spacing, Color color) const;
};
