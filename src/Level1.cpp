#include "Level1.h"

/** @brief Initialises the level: loads textures, builds the U-shaped target, and spawns entities. */
void Level1::initialise(ResourceManager& resources) {
    mCurrentSceneID = SCENE_LEVEL1;
    initialiseLevel(resources, L1_GROUND_WIDTH_PX, GROUND_HEIGHT_PX);
    mBox1Tex = raylib::Texture("assets/box1.png");
    mBox2Tex = raylib::Texture("assets/box2.png");

    // Target: An upside-down U-shaped polygon centered on screen, sitting on the ground surface.
    // Built from two vertical walls and a horizontal top bar, decomposed into 3 convex rects.
    float centerX = toMeters(SCREEN_WIDTH / 2.0f);
    float surfaceY = toMeters(GROUND_HEIGHT_PX - GROUND_RECESS_PX); // top of ground, recessed slightly
    float leftX = centerX - toMeters(L1_TOTAL_WIDTH_PX) / 2.0f;     // outer left edge
    float rightX = centerX + toMeters(L1_TOTAL_WIDTH_PX) / 2.0f;    // outer right edge
    float innerLeftX = leftX + toMeters(L1_WALL_THICKNESS_PX);      // inner left (U mouth)
    float innerRightX = rightX - toMeters(L1_WALL_THICKNESS_PX);    // inner right (U mouth)
    float topY = surfaceY + toMeters(L1_WALL_HEIGHT_PX);            // top of walls
    float barBottomY = topY - toMeters(L1_TOP_BAR_H_PX);            // underside of the top bar

    std::vector<b2Vec2> outline = {
        {leftX, surfaceY},       {innerLeftX, surfaceY}, {innerLeftX, barBottomY}, {innerRightX, barBottomY},
        {innerRightX, surfaceY}, {rightX, surfaceY},     {rightX, topY},           {leftX, topY},
    };

    std::vector<std::vector<b2Vec2>> convexPieces = {
        {{leftX, surfaceY}, {innerLeftX, surfaceY}, {innerLeftX, topY}, {leftX, topY}},     // left wall
        {{innerRightX, surfaceY}, {rightX, surfaceY}, {rightX, topY}, {innerRightX, topY}}, // right wall
        {{leftX, barBottomY}, {rightX, barBottomY}, {rightX, topY}, {leftX, topY}},         // top bar
    };

    mState.target = Target(mState.worldId, outline, convexPieces);

    spawnEntities();
    mPreviousTicks = static_cast<float>(GetTime());
}

/** @brief Spawns three stacked boxes centerd on screen above the ground surface. */
void Level1::spawnEntities() {
    float groundY = SCREEN_HEIGHT - GROUND_HEIGHT_PX; // top of ground in screen coords
    float spawnX = SCREEN_WIDTH / 2.0f;               // all boxes start centerd horizontally
    float boxLayerH = L1_BOX_HALF_H_PX * 2.0f;        // full height of one box layer

    // Stack three boxes on the ground, each shifted up by one full box height.
    // center Y of each = groundY - halfH - (layer * fullH), so the bottom edge rests on the layer below.
    mState.entities.push_back(Entity::createBox(mState.worldId, spawnX, groundY - L1_BOX_HALF_H_PX, L1_BOX1_HALF_W_PX,
                                                L1_BOX_HALF_H_PX, {&mBox1Tex, TEX_PADDING_PX}));
    mState.entities.push_back(Entity::createBox(mState.worldId, spawnX, groundY - L1_BOX_HALF_H_PX - boxLayerH,
                                                L1_BOX2_HALF_W_PX, L1_BOX_HALF_H_PX, {&mBox2Tex, TEX_PADDING_PX}));
    mState.entities.push_back(Entity::createBox(mState.worldId, spawnX, groundY - L1_BOX_HALF_H_PX - boxLayerH * 2,
                                                L1_BOX2_HALF_W_PX, L1_BOX_HALF_H_PX, {&mBox2Tex, TEX_PADDING_PX}));
}
