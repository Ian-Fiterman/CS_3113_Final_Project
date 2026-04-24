#pragma once

#include "Level.h"

constexpr float L1_GROUND_WIDTH_PX = 1000.0f;

constexpr float L1_WALL_THICKNESS_PX = 80.0f;
constexpr float L1_WALL_HEIGHT_PX = 480.0f;
constexpr float L1_TOP_BAR_H_PX = 80.0f;
constexpr float L1_TOTAL_WIDTH_PX = 320.0f;

constexpr float L1_BOX_HALF_H_PX = 30.0f;
constexpr float L1_BOX1_HALF_W_PX = 150.0f;
constexpr float L1_BOX2_HALF_W_PX = 205.0f;

class Level1 : public Level {
public:
    Level1() = default;
    ~Level1() = default;

    void initialise(ResourceManager& resources) override;

private:
    raylib::Texture mBox1Tex;
    raylib::Texture mBox2Tex;

    void spawnEntities() override;
};
