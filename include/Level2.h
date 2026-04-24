#pragma once

#include "Level.h"

constexpr float L2_GROUND_WIDTH_PX = 1000.0f;
constexpr float L2_TRI_LONG_LEG_PX = 400.0f;
constexpr float L2_TRI_SHORT_LEG_PX = 200.0f;
constexpr float L2_INNER_SQUARE_HALF_PX = (L2_TRI_LONG_LEG_PX - L2_TRI_SHORT_LEG_PX) / 2.0f;

class Level2 : public Level {
public:
    Level2() = default;
    ~Level2() = default;

    void initialise(ResourceManager& resources) override;

private:
    raylib::Texture mTexSquare;
    raylib::Texture mTexTriangle;

    void spawnEntities() override;
};
