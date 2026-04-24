#include "Entity.h"

/** @brief Creates a dynamic Box2D body at worldPos with angular damping applied. */
static b2BodyId createBody(b2WorldId worldId, b2Vec2 worldPos) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.angularDamping = ANGULAR_DAMPING;
    bodyDef.position = worldPos;
    return b2CreateBody(worldId, &bodyDef);
}

/** @brief Applies the shared physics material defaults (density, friction, restitution, events) to a shape def. */
static void setShapeDefaults(b2ShapeDef* def) {
    def->density = DENSITY;
    def->material.friction = FRICTION;
    def->material.restitution = RESTITUTION;
    def->enableHitEvents = true;
    def->enableContactEvents = true;
}

/** @brief Copies the vertex array from a b2Polygon into a std::vector. */
static std::vector<b2Vec2> polyVerts(const b2Polygon& poly) {
    std::vector<b2Vec2> verts;
    verts.reserve(poly.count);
    for (int i = 0; i < poly.count; i++)
        verts.push_back(poly.vertices[i]);
    return verts;
}

/** @brief Computes and caches the body-local AABB center offset and pixel extents from the given vertex list. */
void Entity::cacheAABB(const std::vector<b2Vec2>& localVerts) {
    float minX = localVerts[0].x, maxX = localVerts[0].x;
    float minY = localVerts[0].y, maxY = localVerts[0].y;
    for (const auto& v : localVerts) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
    }
    mLocalAABBOffset = {(minX + maxX) / 2.0f, (minY + maxY) / 2.0f};
    mLocalAABBExtents = {toPixels(maxX - minX), toPixels(maxY - minY)};
}

/** @brief Creates a rectangular dynamic entity with the given half-extents in pixels. */
Entity Entity::createBox(b2WorldId worldId, float screenX, float screenY, float halfWidthPx, float halfHeightPx,
                         TextureInfo texInfo) {
    Entity entity;
    entity.mBodyId = createBody(worldId, toWorld({screenX, screenY}));
    entity.mTexInfo = texInfo;

    b2Polygon box = b2MakeBox(toMeters(halfWidthPx), toMeters(halfHeightPx));
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    setShapeDefaults(&shapeDef);
    entity.mShapeId = b2CreatePolygonShape(entity.mBodyId, &shapeDef, &box);

    entity.cacheAABB(polyVerts(box));
    return entity;
}

/** @brief Creates a circular dynamic entity with the given radius in pixels. */
Entity Entity::createCircle(b2WorldId worldId, float screenX, float screenY, float radiusPx, TextureInfo texInfo) {
    Entity entity;
    entity.mBodyId = createBody(worldId, toWorld({screenX, screenY}));
    entity.mTexInfo = texInfo;

    b2Circle circle = {{0.0f, 0.0f}, toMeters(radiusPx)};
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    setShapeDefaults(&shapeDef);
    entity.mShapeId = b2CreateCircleShape(entity.mBodyId, &shapeDef, &circle);

    // Symmetric shape - offset is zero; extents follow directly from radius.
    entity.mLocalAABBOffset = {0.0f, 0.0f};
    entity.mLocalAABBExtents = {radiusPx * 2.0f, radiusPx * 2.0f};
    return entity;
}

// This is currently unused but I used it earlier to test shapes. Left it in here.
/** @brief Creates a regular n-gon by placing sides vertices evenly around a circle of radiusPx. */
Entity Entity::createRegularPolygon(b2WorldId worldId, float screenX, float screenY, int sides, float radiusPx,
                                    TextureInfo texInfo) {
    const float radiusM = toMeters(radiusPx);
    std::vector<b2Vec2> verts(sides);
    for (int i = 0; i < sides; i++) {
        const float angle = (2.0f * PI * i) / sides;
        verts[i] = {radiusM * cosf(angle), radiusM * sinf(angle)};
    }
    return createConvexPolygon(worldId, screenX, screenY, verts, texInfo);
}

/** @brief Creates a convex polygon entity from world-space vertices, recenterd on their centroid. */
Entity Entity::createConvexPolygon(b2WorldId worldId, float screenX, float screenY,
                                   const std::vector<b2Vec2>& worldVerts, TextureInfo texInfo) {
    Entity entity;
    entity.mBodyId = createBody(worldId, toWorld({screenX, screenY}));
    entity.mTexInfo = texInfo;

    // Recenter verts around their centroid so the body origin sits at the shape center.
    const b2Vec2 centroid = centroidOf(worldVerts);
    const int count = static_cast<int>(worldVerts.size());
    std::vector<b2Vec2> localV(count);
    for (int i = 0; i < count; i++)
        localV[i] = {worldVerts[i].x - centroid.x, worldVerts[i].y - centroid.y};
    entity.mLocalVerts = localV;

    b2Hull hull = b2ComputeHull(localV.data(), count);
    b2Polygon poly = b2MakePolygon(&hull, 0.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    setShapeDefaults(&shapeDef);
    entity.mShapeId = b2CreatePolygonShape(entity.mBodyId, &shapeDef, &poly);

    entity.cacheAABB(entity.mLocalVerts);
    return entity;
}

/** @brief Renders the entity as a texture with an optional physics outline drawn on top. */
void Entity::render(Color color, bool drawOutline, raylib::Shader* shader) const {
    const b2ShapeType type = b2Shape_GetType(mShapeId);
    if (mTexInfo.texture) renderTexture(shader);
    if (!mTexInfo.texture || drawOutline) {
        if (type == b2_polygonShape)
            renderPolygonOutline(color);
        else if (type == b2_circleShape)
            renderCircleOutline(color);
    }
}

/** @brief Draws the polygon edges as lines in the given color. */
void Entity::renderPolygonOutline(Color color) const {
    const b2Transform xf = b2Body_GetTransform(mBodyId);
    const b2Polygon poly = b2Shape_GetPolygon(mShapeId);
    for (int i = 0; i < poly.count; i++) {
        Vector2 a = toScreen(b2TransformPoint(xf, poly.vertices[i]));
        Vector2 b = toScreen(b2TransformPoint(xf, poly.vertices[(i + 1) % poly.count]));
        DrawLineEx(a, b, 2.0f, color);
    }
}

/** @brief Draws a circle outline in the given color. */
void Entity::renderCircleOutline(Color color) const {
    const Vector2 center = toScreen(b2Body_GetPosition(mBodyId));
    const float radiusPx = toPixels(b2Shape_GetCircle(mShapeId).radius);
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), radiusPx, color);
}

/** @brief Draws the entity texture scaled and rotated to match the physics body transform.
 *         An optional shader is activated around the draw call. */
void Entity::renderTexture(raylib::Shader* shader) const {
    const b2Rot rot = b2Body_GetRotation(mBodyId);
    const float angleDeg = b2Rot_GetAngle(rot) * RAD2DEG;
    const Vector2 center = toScreen(b2Body_GetPosition(mBodyId));

    // Rotate the cached local AABB offset into screen space.
    // rot.c and rot.s are the cosine and sine from Box2D - no extra trig needed.
    const Vector2 aabbCenter = {
        center.x + toPixels(mLocalAABBOffset.x * rot.c - mLocalAABBOffset.y * rot.s),
        center.y - toPixels(mLocalAABBOffset.x * rot.s + mLocalAABBOffset.y * rot.c),
    };

    const float pad = mTexInfo.texturePaddingPx;
    const float drawW = mLocalAABBExtents.x + pad * 2.0f;
    const float drawH = mLocalAABBExtents.y + pad * 2.0f;

    // Negative source dimension flips the texture along that axis.
    const float srcX = mTexInfo.textureFlipX ? static_cast<float>(mTexInfo.texture->width) : 0.0f;
    const float srcW = mTexInfo.textureFlipX ? -static_cast<float>(mTexInfo.texture->width) :
                                               static_cast<float>(mTexInfo.texture->width);
    const float srcY = mTexInfo.textureFlipY ? static_cast<float>(mTexInfo.texture->height) : 0.0f;
    const float srcH = mTexInfo.textureFlipY ? -static_cast<float>(mTexInfo.texture->height) :
                                               static_cast<float>(mTexInfo.texture->height);

    const Rectangle src = {srcX, srcY, srcW, srcH};
    const Rectangle dest = {aabbCenter.x, aabbCenter.y, drawW, drawH};
    const Vector2 orig = {drawW / 2.0f, drawH / 2.0f}; // pivot at draw center

    if (shader) shader->BeginMode();
    mTexInfo.texture->Draw(src, dest, orig, -(angleDeg + mTexInfo.textureAngleOffset), WHITE);
    if (shader) EndShaderMode();
}

/** @brief Returns a pointer to the first entity whose shape contains worldPos, or nullptr if none. */
Entity* entityAtWorldPoint(std::vector<Entity>& entities, b2Vec2 worldPos) {
    for (auto& entity : entities)
        if (b2Shape_TestPoint(entity.shapeId(), worldPos)) return &entity;
    return nullptr;
}

/** @brief Returns the arithmetic mean of the vertex positions (non-area-weighted centroid). */
b2Vec2 centroidOf(const std::vector<b2Vec2>& verts) {
    b2Vec2 centroid = {0.0f, 0.0f};
    for (const auto& v : verts) {
        centroid.x += v.x;
        centroid.y += v.y;
    }
    const float n = static_cast<float>(verts.size());
    return {centroid.x / n, centroid.y / n};
}
