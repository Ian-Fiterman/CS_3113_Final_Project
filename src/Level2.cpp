#include "Level2.h"

#include <cmath>

/** @brief Initialises the level: loads textures, builds the square target, and spawns entities. */
void Level2::initialise(ResourceManager& resources) {
    mCurrentSceneID = SCENE_LEVEL2;
    initialiseLevel(resources, L2_GROUND_WIDTH_PX, GROUND_HEIGHT_PX);

    mTexSquare = raylib::Texture("assets/square1.png");
    mTexTriangle = raylib::Texture("assets/triangle1.png");

    float shortLeg = toMeters(L2_TRI_SHORT_LEG_PX);
    float longLeg = toMeters(L2_TRI_LONG_LEG_PX);
    float centerX = toMeters(SCREEN_WIDTH / 2.0f);
    float surfaceY = toMeters(GROUND_HEIGHT_PX);

    // Target: a padded square whose side equals the hypotenuse of one triangle,
    // centerd on screen and sitting on the ground surface.
    float hypotenuse = sqrtf(shortLeg * shortLeg + longLeg * longLeg);
    float targetPad = toMeters(TARGET_PADDING_PX);
    float targetBottom = surfaceY - toMeters(GROUND_RECESS_PX);

    std::vector<b2Vec2> outline = {
        {centerX - hypotenuse / 2.0f - targetPad, targetBottom},
        {centerX + hypotenuse / 2.0f + targetPad, targetBottom},
        {centerX + hypotenuse / 2.0f + targetPad, surfaceY + hypotenuse + targetPad},
        {centerX - hypotenuse / 2.0f - targetPad, surfaceY + hypotenuse + targetPad},
    };
    mState.target = Target(mState.worldId, outline, {outline});

    spawnEntities();
    mPreviousTicks = static_cast<float>(GetTime());
}

/** @brief Spawns four pinwheel triangles and the inner square centerd on screen. */
void Level2::spawnEntities() {
    float shortLeg = toMeters(L2_TRI_SHORT_LEG_PX);
    float longLeg = toMeters(L2_TRI_LONG_LEG_PX);
    float centerX = toMeters(SCREEN_WIDTH / 2.0f);
    float surfaceY = toMeters(GROUND_HEIGHT_PX);

    // Six shared corners define two A x B rectangles sitting side by side on the ground surface.
    // Each rectangle is split diagonally into two triangles that share an edge.
    b2Vec2 bottomLeft = {centerX - shortLeg, surfaceY};
    b2Vec2 bottomCenter = {centerX, surfaceY};
    b2Vec2 bottomRight = {centerX + shortLeg, surfaceY};
    b2Vec2 topLeft = {centerX - shortLeg, surfaceY + longLeg};
    b2Vec2 topCenter = {centerX, surfaceY + longLeg};
    b2Vec2 topRight = {centerX + shortLeg, surfaceY + longLeg};

    // T1/T3: base orientation - right angle at bottom-left / bottom-center.
    // T2/T4: flipped 180 deg to fill the other half of each rectangle.
    spawnConvexAtCentroid({bottomLeft, bottomCenter, topLeft}, {&mTexTriangle, TEX_PADDING_PX, false, false});    // T1
    spawnConvexAtCentroid({topCenter, topLeft, bottomCenter}, {&mTexTriangle, TEX_PADDING_PX, true, true});       // T2
    spawnConvexAtCentroid({bottomCenter, bottomRight, topCenter}, {&mTexTriangle, TEX_PADDING_PX, false, false}); // T3
    spawnConvexAtCentroid({topRight, topCenter, bottomRight}, {&mTexTriangle, TEX_PADDING_PX, true, true});       // T4

    // Inner square spawns directly above the two rectangles. Y offset by the full triangle height
    // plus one half-side so its bottom edge clears the top of the rectangle stack.
    float innerSpawnY = SCREEN_HEIGHT - GROUND_HEIGHT_PX - L2_TRI_LONG_LEG_PX - L2_INNER_SQUARE_HALF_PX;
    mState.entities.push_back(Entity::createBox(mState.worldId, SCREEN_WIDTH / 2.0f, innerSpawnY,
                                                L2_INNER_SQUARE_HALF_PX, L2_INNER_SQUARE_HALF_PX,
                                                {&mTexSquare, TEX_PADDING_PX, false, false, 0.0f}));
}
