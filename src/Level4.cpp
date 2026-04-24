#include "Level4.h"

#include <cmath>

/** @brief Initialises the level: loads textures, builds the target rectangle, and spawns entities. */
void Level4::initialise(ResourceManager& resources) {
    mCurrentSceneID = SCENE_LEVEL4;
    initialiseLevel(resources, L4_GROUND_WIDTH_PX, GROUND_HEIGHT_PX);

    mTexTrapezoid = raylib::Texture("assets/trapezoid.png");
    mTexTriangle = raylib::Texture("assets/triangle2.png");
    mCircle = raylib::Texture("assets/circle.png");

    const float sq5 = sqrtf(5.0f);

    float screenCenterX = SCREEN_WIDTH / 2.0f;
    float groundSurfaceY = SCREEN_HEIGHT - GROUND_HEIGHT_PX;

    // Trapezoid half-widths follow from golden ratio geometry: 25*(3 + sqrt(5)) and 25*(5 + sqrt(5)).
    float trapBotHalf = 75.0f + 25.0f * sq5;
    float trapTopHalf = 125.0f + 25.0f * sq5;

    // Target: bounding rectangle of the solved arrangement.
    // Width spans the two triangle bases (2*TRI_SHORT) plus the gap between circle stacks (solvedGap).
    // Height spans from the ground up to the triangle apex (TRI_LONG).
    float solvedGap = L4_CIRCLE_RADIUS_PX * (1.0f + sq5);  // center-to-center gap = d = r*(1 + sqrt(5))
    float solvedHalf = L4_TRI_SHORT_PX + solvedGap / 2.0f; // half the total solved width

    float targetLeft = screenCenterX - solvedHalf - TARGET_PADDING_PX;
    float targetRight = screenCenterX + solvedHalf + TARGET_PADDING_PX;
    float targetTop = groundSurfaceY - L4_TRI_LONG_PX - TARGET_PADDING_PX;
    float targetBottom = groundSurfaceY + GROUND_RECESS_PX;

    std::vector<b2Vec2> outline = {
        toWorld({targetLeft, targetBottom}),
        toWorld({targetRight, targetBottom}),
        toWorld({targetRight, targetTop}),
        toWorld({targetLeft, targetTop}),
    };
    mState.target = Target(mState.worldId, outline, {outline});

    spawnEntities();
    mPreviousTicks = static_cast<float>(GetTime());
}

/** @brief Spawns the trapezoid, two triangles, and six circles using golden ratio proportions. */
void Level4::spawnEntities() {
    const float sq5 = sqrtf(5.0f);
    const float sq3 = sqrtf(3.0f);

    float screenCenterX = SCREEN_WIDTH / 2.0f;
    float groundSurfaceY = SCREEN_HEIGHT - GROUND_HEIGHT_PX;

    // Trapezoid half-widths: bottom wider than top, both derived from 25*(n + sqrt(5)).
    float trapBotHalf = 75.0f + 25.0f * sq5;
    float trapTopHalf = 125.0f + 25.0f * sq5;
    float trapBotY = groundSurfaceY;
    float trapTopY = groundSurfaceY - L4_TRAP_HEIGHT_PX;

    // Trapezoid sits with its bottom edge on the ground, centerd horizontally.
    spawnConvexAtCentroid(
        {
            toWorld({screenCenterX - trapBotHalf, trapBotY}),
            toWorld({screenCenterX + trapBotHalf, trapBotY}),
            toWorld({screenCenterX + trapTopHalf, trapTopY}),
            toWorld({screenCenterX - trapTopHalf, trapTopY}),
        },
        {&mTexTrapezoid, TEX_PADDING_PX});

    // Left triangle: base flush with the trapezoid's top-left corner, short leg horizontal,
    // long leg vertical, right angle at bottom-right.
    float leftTriBaseX = screenCenterX - trapTopHalf;
    spawnConvexAtCentroid(
        {
            toWorld({leftTriBaseX, trapTopY}),
            toWorld({leftTriBaseX + L4_TRI_SHORT_PX, trapTopY}),
            toWorld({leftTriBaseX + L4_TRI_SHORT_PX, trapTopY - L4_TRI_LONG_PX}),
        },
        {&mTexTriangle, TEX_PADDING_PX, true, false});

    // Right triangle: mirror of left, flush with the trapezoid's top-right corner,
    // right angle at bottom-left.
    float rightTriBaseX = screenCenterX + trapTopHalf;
    spawnConvexAtCentroid(
        {
            toWorld({rightTriBaseX - L4_TRI_SHORT_PX, trapTopY}),
            toWorld({rightTriBaseX, trapTopY}),
            toWorld({rightTriBaseX - L4_TRI_SHORT_PX, trapTopY - L4_TRI_LONG_PX}),
        },
        {&mTexTriangle, TEX_PADDING_PX});

    // Each circle stack: two circles resting on the ground touching each other,
    // third circle balanced on top. center Y of bottom circles = groundY - r so they sit on the surface.
    // Top circle center Y = bottomY - r*sqrt(3), the exact height where a circle of radius r rests
    // in the groove between two tangent circles of the same radius.
    float circleSurfaceY = groundSurfaceY - L4_CIRCLE_RADIUS_PX;
    float circleStackTopY = circleSurfaceY - L4_CIRCLE_RADIUS_PX * sq3;

    // Stack centers are offset from screen center by 2 * trapBotHalf (the full bottom width of the trapezoid).
    float leftStackCX = screenCenterX - 2.0f * trapBotHalf;
    float rightStackCX = screenCenterX + 2.0f * trapBotHalf;

    spawnCircleAt(leftStackCX - L4_CIRCLE_RADIUS_PX, circleSurfaceY, L4_CIRCLE_RADIUS_PX, {&mCircle, TEX_PADDING_PX});
    spawnCircleAt(leftStackCX + L4_CIRCLE_RADIUS_PX, circleSurfaceY, L4_CIRCLE_RADIUS_PX, {&mCircle, TEX_PADDING_PX});
    spawnCircleAt(leftStackCX, circleStackTopY, L4_CIRCLE_RADIUS_PX, {&mCircle, TEX_PADDING_PX});

    spawnCircleAt(rightStackCX - L4_CIRCLE_RADIUS_PX, circleSurfaceY, L4_CIRCLE_RADIUS_PX, {&mCircle, TEX_PADDING_PX});
    spawnCircleAt(rightStackCX + L4_CIRCLE_RADIUS_PX, circleSurfaceY, L4_CIRCLE_RADIUS_PX, {&mCircle, TEX_PADDING_PX});
    spawnCircleAt(rightStackCX, circleStackTopY, L4_CIRCLE_RADIUS_PX, {&mCircle, TEX_PADDING_PX});
}
