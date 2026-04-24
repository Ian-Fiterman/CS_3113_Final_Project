#include "Level3.h"

#include <cmath>

// Pushes a vertex outward from the centroid along the direction (localX, localY - centroidLocalY).
// Used to uniformly pad each vertex of the equilateral target triangle away from its edges.
static b2Vec2 outsetTriangleVertex(float localX, float localY, b2Vec2 worldOrigin, float centroidLocalY,
                                   float vertexOutset) {
    float dx = localX, dy = localY - centroidLocalY;
    float len = sqrtf(dx * dx + dy * dy);
    return {worldOrigin.x + localX + (dx / len) * vertexOutset, worldOrigin.y + localY + (dy / len) * vertexOutset};
}

// Translates a square-local offset (nx, ny) into Box2D world space given the square's center.
static b2Vec2 squareLocal(float nx, float ny, b2Vec2 squareCenter) {
    return {squareCenter.x + nx, squareCenter.y + ny};
}

/** @brief Initialises the level: loads textures, builds the equilateral triangle target, and spawns entities. */
void Level3::initialise(ResourceManager& resources) {
    mCurrentSceneID = SCENE_LEVEL3;
    initialiseLevel(resources, L3_GROUND_WIDTH_PX, GROUND_HEIGHT_PX);

    mTexTopRight = raylib::Texture("assets/p1.png");
    mTexBotRight = raylib::Texture("assets/p2.png");
    mTexBotLeft = raylib::Texture("assets/p3.png");
    mTexTopLeft = raylib::Texture("assets/p4.png");

    float centerX = toMeters(SCREEN_WIDTH / 2.0f);
    float surfaceY = toMeters(GROUND_HEIGHT_PX);
    b2Vec2 worldOrigin = {centerX, surfaceY};

    // Target: equilateral triangle whose side length comes from the Haberdasher dissection relation
    // t = squareSide * 2 / fourthRoot(3). Each vertex is then outset by pad * 2 (= pad / sin(60 deg))
    // so all three edges are uniformly inset by the padding distance.
    float triSide = toMeters(L3_SQUARE_SIDE_PX) * 2.0f / powf(3.0f, 0.25f);
    float triHeight = triSide * sqrtf(3.0f) / 2.0f;
    float triCentroidY = triHeight / 3.0f; // centroid sits 1/3 of the way up from the base
    float vertexOutset = toMeters(TARGET_PADDING_PX) * 2.0f;

    b2Vec2 targetLeft = outsetTriangleVertex(-triSide / 2.0f, 0.0f, worldOrigin, triCentroidY, vertexOutset);
    b2Vec2 targetRight = outsetTriangleVertex(triSide / 2.0f, 0.0f, worldOrigin, triCentroidY, vertexOutset);
    b2Vec2 targetApex = outsetTriangleVertex(0.0f, triHeight, worldOrigin, triCentroidY, vertexOutset);

    // Snap base vertices into the ground to avoid a visible gap at the seam.
    targetLeft.y = surfaceY - toMeters(GROUND_RECESS_PX);
    targetRight.y = surfaceY - toMeters(GROUND_RECESS_PX);

    std::vector<b2Vec2> outline = {targetLeft, targetRight, targetApex};
    mState.target = Target(mState.worldId, outline, {outline});

    spawnEntities();
    mPreviousTicks = static_cast<float>(GetTime());
}

// This math took forever to figure out :(
/** @brief Spawns the four Haberdasher dissection pieces assembled as a square on the ground. */
void Level3::spawnEntities() {
    float centerX = toMeters(SCREEN_WIDTH / 2.0f);
    float surfaceY = toMeters(GROUND_HEIGHT_PX);
    float halfSide = toMeters(L3_SQUARE_SIDE_PX) / 2.0f;

    b2Vec2 squareCenter = {centerX, surfaceY + halfSide}; // square sits with its base on the ground

    // Corners of the bounding square
    b2Vec2 sqTL = squareLocal(-halfSide, halfSide, squareCenter);
    b2Vec2 sqTR = squareLocal(halfSide, halfSide, squareCenter);
    b2Vec2 sqBR = squareLocal(halfSide, -halfSide, squareCenter);
    b2Vec2 sqBL = squareLocal(-halfSide, -halfSide, squareCenter);

    // Interior cut points derived from the Dudeney dissection, rotated into spawn space.
    b2Vec2 spawnMidTop = squareLocal(0.0f, halfSide, squareCenter);
    b2Vec2 spawnMidBot = squareLocal(0.0f, -halfSide, squareCenter);
    b2Vec2 spawnEdgeRight = squareLocal(halfSide, -L3_J_EDGE_X_NORM * halfSide, squareCenter);
    b2Vec2 spawnEdgeLeft = squareLocal(-halfSide, L3_J_EDGE_X_NORM * halfSide, squareCenter);
    b2Vec2 spawnInterior = squareLocal(L3_INTERIOR_Y_NORM * halfSide, -L3_INTERIOR_X_NORM * halfSide, squareCenter);

    spawnConvexAtCentroid({sqTR, spawnEdgeRight, spawnInterior, spawnMidTop}, {&mTexTopRight, TEX_PADDING_PX});
    spawnConvexAtCentroid({spawnEdgeRight, sqBR, spawnMidBot}, {&mTexBotRight, TEX_PADDING_PX});
    spawnConvexAtCentroid({spawnInterior, spawnMidBot, sqBL, spawnEdgeLeft}, {&mTexBotLeft, TEX_PADDING_PX});
    spawnConvexAtCentroid({spawnMidTop, spawnInterior, spawnEdgeLeft, sqTL}, {&mTexTopLeft, TEX_PADDING_PX});
}
