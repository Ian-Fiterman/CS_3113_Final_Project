#pragma once

#include "box2d/box2d.h"
#include "raylib-cpp.hpp"
#include "raymath.h"
#include "Scene.h"

#include <cmath>
#include <vector>

// Physics material defaults applied to every entity shape
constexpr float SCALE = 80.0f; // pixels per meter
// ^^^ I was so tempted to make this 67.0f
constexpr float DENSITY = 1.0f;
constexpr float FRICTION = 0.5f;
constexpr float RESTITUTION = 0.3f;
constexpr float ANGULAR_DAMPING = 0.1f;

// Coordinate conversion
// Box2D: origin bottom-left, Y up,  angles in radians CCW
// Raylib: origin top-left,  Y down, angles in degrees  CW
constexpr float toPixels(float meters) {
    return meters * SCALE;
}

constexpr float toMeters(float pixels) {
    return pixels / SCALE;
}

inline Vector2 toScreen(b2Vec2 world) {
    return {toPixels(world.x), SCREEN_HEIGHT - toPixels(world.y)};
}

inline b2Vec2 toWorld(Vector2 screen) {
    return {toMeters(screen.x), toMeters(SCREEN_HEIGHT - screen.y)};
}

// Per-entity texture and rendering configuration
struct TextureInfo {
    const raylib::Texture* texture = nullptr;
    float texturePaddingPx = 0.0f;
    bool textureFlipX = false;
    bool textureFlipY = false;
    float textureAngleOffset = 0.0f;
};

class Entity {
public:
    // Factory methods - spawn position in screen coordinates (pixels)
    static Entity createBox(b2WorldId worldId, float screenX, float screenY, float halfWidthPx, float halfHeightPx,
                            TextureInfo texInfo = {});

    static Entity createCircle(b2WorldId worldId, float screenX, float screenY, float radiusPx,
                               TextureInfo texInfo = {});

    static Entity createRegularPolygon(b2WorldId worldId, float screenX, float screenY, int sides, float radiusPx,
                                       TextureInfo texInfo = {});

    static Entity createConvexPolygon(b2WorldId worldId, float screenX, float screenY,
                                      const std::vector<b2Vec2>& worldVerts, TextureInfo texInfo = {});

    void render(Color color, bool drawOutline, raylib::Shader* shader = nullptr) const;

    // Physics handles
    b2BodyId bodyId() const { return mBodyId; }

    b2ShapeId shapeId() const { return mShapeId; }

    // Local vertex and AABB data
    const std::vector<b2Vec2>& localVerts() const { return mLocalVerts; }

    b2Vec2 localAABBExtents() const { return mLocalAABBExtents; }

    void setLocalAABBOffset(b2Vec2 offset) { mLocalAABBOffset = offset; }

    void setLocalAABBExtents(b2Vec2 extents) { mLocalAABBExtents = extents; }

private:
    Entity() = default;

    void cacheAABB(const std::vector<b2Vec2>& localVerts);
    void renderPolygonOutline(Color color) const;
    void renderCircleOutline(Color color) const;
    void renderTexture(raylib::Shader* shader) const;

    b2BodyId mBodyId = b2_nullBodyId;
    b2ShapeId mShapeId = b2_nullShapeId;

    std::vector<b2Vec2> mLocalVerts;

    // Cached body-local AABB used for texture drawing (offset in meters, extents in pixels)
    b2Vec2 mLocalAABBOffset = {0.0f, 0.0f};
    b2Vec2 mLocalAABBExtents = {0.0f, 0.0f};

    TextureInfo mTexInfo;
};

// Hit-test the entity list against a Box2D world-space point
Entity* entityAtWorldPoint(std::vector<Entity>& entities, b2Vec2 worldPos);

// Arithmetic centroid of a vertex list
b2Vec2 centroidOf(const std::vector<b2Vec2>& verts);
