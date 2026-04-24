#pragma once

#include "Entity.h"

#include <vector>

// Fun fact: This class was originally called "Silhouette" but I literally could never spell that.
// I LITERALLY MISSPELLED IT TYPING THIS COMMENT LMAO
class Target {
public:
    Target() = default;
    Target(b2WorldId worldId, const std::vector<b2Vec2>& outlineVertices,
           const std::vector<std::vector<b2Vec2>>& convexPieces);

    ~Target();

    // Disable copy semantics; Box2D body and shapes are not copyable
    Target(const Target&) = delete;
    Target& operator=(const Target&) = delete;

    // Enable move semantics; transfers Box2D ownership, nulls the source
    Target(Target&& other) noexcept;
    Target& operator=(Target&& other) noexcept;

    void update(const std::vector<Entity>& entities);

    void render(Color color) const;

    bool allInside() const { return mAllInside; }

private:
    bool mAllInside = false;
    b2WorldId mWorldId;
    b2BodyId mBodyId = b2_nullBodyId;
    std::vector<b2ShapeId> mShapeIds;
    std::vector<b2Vec2> mOutlineVertices;

    bool pointInsideAllPieces(b2Vec2 pt) const;

    bool circleInsideAllPieces(b2Vec2 center, float radius) const;

    void destroy();
};