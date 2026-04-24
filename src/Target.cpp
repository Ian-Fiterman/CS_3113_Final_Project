#include "Target.h"

#include <cmath>

constexpr float OUTLINE_LINE_WIDTH = 3.0f;

/**
 * @brief Constructs the target region from an outline polygon and a set of convex decomposition pieces.
 *        Creates a static Box2D body with one sensor shape per convex piece.
 * @param worldId        Box2D world to create the body in.
 * @param outlineVertices Vertices of the visual outline polygon in world space.
 * @param convexPieces   Convex decomposition of the target region for point containment tests.
 */
Target::Target(b2WorldId worldId, const std::vector<b2Vec2>& outlineVertices,
               const std::vector<std::vector<b2Vec2>>& convexPieces) :
    mWorldId(worldId), mOutlineVertices(outlineVertices) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    mBodyId = b2CreateBody(worldId, &bodyDef);

    for (const auto& piece : convexPieces) {
        const int count = static_cast<int>(piece.size());
        b2Hull hull = b2ComputeHull(piece.data(), count);
        b2Polygon poly = b2MakePolygon(&hull, 0.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.isSensor = true;
        shapeDef.enableSensorEvents = true;
        mShapeIds.push_back(b2CreatePolygonShape(mBodyId, &shapeDef, &poly));
    }
}

/**
 * @brief Destructor. Releases all Box2D resources owned by this target.
 */
Target::~Target() {
    destroy();
}

/**
 * @brief Destroys the Box2D body and clears all cached shape and vertex data.
 */
void Target::destroy() {
    if (b2Body_IsValid(mBodyId)) {
        b2DestroyBody(mBodyId);
        mBodyId = b2_nullBodyId;
        mShapeIds.clear();
        mOutlineVertices.clear();
    }
}

/**
 * @brief Move constructor. Transfers ownership of Box2D resources from other.
 */
Target::Target(Target&& other) noexcept :
    mWorldId(other.mWorldId), mBodyId(other.mBodyId), mShapeIds(std::move(other.mShapeIds)),
    mOutlineVertices(std::move(other.mOutlineVertices)), mAllInside(other.mAllInside) {
    other.mBodyId = b2_nullBodyId; // prevent double-destroy
}

/**
 * @brief Move assignment operator. Releases existing resources then transfers from other.
 */
Target& Target::operator=(Target&& other) noexcept {
    if (this != &other) {
        destroy();
        mWorldId = other.mWorldId;
        mBodyId = other.mBodyId;
        mShapeIds = std::move(other.mShapeIds);
        mOutlineVertices = std::move(other.mOutlineVertices);
        mAllInside = other.mAllInside;
        other.mBodyId = b2_nullBodyId;
    }
    return *this;
}

/**
 * @brief Tests whether a point lies inside any one of the convex sensor pieces.
 * @param pt Point in Box2D world space.
 * @return True if the point is inside at least one piece.
 */
bool Target::pointInsideAllPieces(b2Vec2 pt) const {
    for (auto shapeId : mShapeIds) {
        if (b2Shape_TestPoint(shapeId, pt)) return true;
    }
    return false;
}

/**
 * @brief Tests whether a circle is fully contained within any one of the convex sensor pieces.
 *
 * A circle is fully inside a convex polygon when its center is inside and its signed distance
 * to every edge is at least the radius. Box2D stores pre-computed outward edge normals in b2Polygon,
 * so the signed distance from center to edge i is:
 *   d = dot(normal[i], center - vertex[i])
 * The circle fits iff d <= -radius for all edges (center is at least radius inside each edge).
 *
 * @param center Circle center in Box2D world space.
 * @param radius Circle radius in meters.
 * @return True if the circle fits fully inside at least one piece.
 */
bool Target::circleInsideAllPieces(b2Vec2 center, float radius) const {
    for (auto shapeId : mShapeIds) {
        const b2Polygon poly = b2Shape_GetPolygon(shapeId);
        const b2Transform xf = b2Body_GetTransform(mBodyId);
        const b2Vec2 localCenter = b2InvTransformPoint(xf, center); // transform to body-local space

        bool insideThisPiece = true;
        for (int i = 0; i < poly.count; i++) {
            const float d = b2Dot(poly.normals[i], b2Sub(localCenter, poly.vertices[i]));
            if (d > -radius) {
                insideThisPiece = false;
                break;
            }
        }

        if (insideThisPiece) return true;
    }
    return false;
}

/**
 * @brief Checks whether all entities are at rest and fully contained within the target region.
 *        Sets mAllInside to true only if every entity passes its containment test.
 * @param entities List of entities to test.
 */
void Target::update(const std::vector<Entity>& entities) {
    mAllInside = false;

    for (const auto& entity : entities) {
        if (b2Body_IsAwake(entity.bodyId())) return; // entity still moving

        switch (b2Shape_GetType(entity.shapeId())) {
        case b2_polygonShape : {
            const b2Polygon poly = b2Shape_GetPolygon(entity.shapeId());
            const b2Transform xf = b2Body_GetTransform(entity.bodyId());
            for (int i = 0; i < poly.count; i++) {
                if (!pointInsideAllPieces(b2TransformPoint(xf, poly.vertices[i]))) return;
            }
            break;
        }
        case b2_circleShape : {
            const b2Vec2 center = b2Body_GetPosition(entity.bodyId());
            const float radius = b2Shape_GetCircle(entity.shapeId()).radius;
            if (!circleInsideAllPieces(center, radius)) return;
            break;
        }
        default :
            break;
        }
    }

    mAllInside = true;
}

/**
 * @brief Draws the target outline as a closed polygon.
 * @param color Line color.
 */
void Target::render(Color color) const {
    const int n = static_cast<int>(mOutlineVertices.size());
    for (int i = 0; i < n; i++) {
        const Vector2 a = toScreen(mOutlineVertices[i]);
        const Vector2 b = toScreen(mOutlineVertices[(i + 1) % n]);
        DrawLineEx(a, b, OUTLINE_LINE_WIDTH, color);
    }
}