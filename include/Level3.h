#pragma once

#include "Level.h"

constexpr float L3_INTERIOR_X_NORM = 0.4381707255f;
constexpr float L3_INTERIOR_Y_NORM = 0.4909847666f;
constexpr float L3_J_EDGE_X_NORM = -0.14429f; // closed-form from Dudeney dissection geometry

constexpr float L3_GROUND_WIDTH_PX = 1000.0f;
constexpr float L3_SQUARE_SIDE_PX = 480.0f; // side of the bounding square (6m)

class Level3 : public Level {
public:
    Level3() = default;
    ~Level3() = default;

    void initialise(ResourceManager& resources) override;

private:
    raylib::Texture mTexTopRight;
    raylib::Texture mTexBotRight;
    raylib::Texture mTexBotLeft;
    raylib::Texture mTexTopLeft;

    void spawnEntities() override;
};
