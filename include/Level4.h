#pragma once

#include "Level.h"

constexpr float L4_GROUND_WIDTH_PX = 1200.0f;
constexpr float L4_TRI_SHORT_PX = 150.0f;
constexpr float L4_TRI_LONG_PX = 300.0f;
constexpr float L4_TRAP_HEIGHT_PX = 100.0f;
constexpr float L4_CIRCLE_RADIUS_PX = 50.0f;

class Level4 : public Level {
public:
    Level4() = default;
    ~Level4() = default;

    void initialise(ResourceManager& resources) override;

private:
    raylib::Texture mTexTriangle;
    raylib::Texture mTexTrapezoid;
    raylib::Texture mCircle;

    void spawnEntities() override;
};
