#pragma once
#include <Entity.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <limits>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <utils.h>

enum class ColliderType {
    AABB,
    OBB,
    Convex
};

struct ColliderAABB {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};
};

struct CollisionMTV {
    glm::vec3 mtv{0.0f};
    glm::vec3 normal{0.0f};
    float penetration{0.0f};
};

class Collider : public Entity {
public:
    using Entity::Entity;
    virtual ~Collider() = default;
    virtual ColliderType getColliderType() const = 0;
    virtual ColliderAABB getWorldAABB() const = 0;
    virtual bool intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos = glm::vec3(0.0f), const glm::vec3& deltaRot = glm::vec3(0.0f)) const = 0;
    glm::vec3 intersects(const Collider& other, const glm::vec3& deltaPos = glm::vec3(0.0f), const glm::vec3& deltaRot = glm::vec3(0.0f)) const {
        CollisionMTV res{};
        if (intersectsMTV(other, res, deltaPos, deltaRot)) return res.mtv;
        return glm::vec3(0.0f);
    }
protected:
    static std::array<glm::vec3, 8> buildOBBCorners(const glm::mat4& transform, const glm::vec3& half);
    static ColliderAABB aabbFromCorners(const std::array<glm::vec3, 8>& corners);
    static void projectOntoAxis(const std::array<glm::vec3, 8>& corners, const glm::vec3& axis, float& min, float& max);
    static std::array<glm::vec3,8> cornersFromAABB(const ColliderAABB& a);
    static bool aabbOverlapMTV(const ColliderAABB& a, const ColliderAABB& b, CollisionMTV& out);
    static bool aabbIntersects(const ColliderAABB& a, const ColliderAABB& b, float margin = 0.0f);
    static glm::vec3 normalizeOrZero(const glm::vec3& v);
    static void addAxisUnique(std::vector<glm::vec3>& axes, const glm::vec3& axis);
    static void projectVertsOntoAxis(const std::vector<glm::vec3>& verts, const glm::vec3& axis, float& mn, float& mx, const glm::vec3& offset = glm::vec3(0.0f));
    static bool satMTV(const std::vector<glm::vec3>& vertsA, const std::vector<glm::vec3>& faceAxesA, const std::vector<glm::vec3>& edgeDirsA, const std::vector<glm::vec3>& vertsB, const std::vector<glm::vec3>& faceAxesB, const std::vector<glm::vec3>& edgeDirsB, const glm::vec3& centerDelta, CollisionMTV& out, const glm::vec3& offsetA = glm::vec3(0.0f), const glm::vec3& offsetB = glm::vec3(0.0f));
    static void buildConvexData(const std::vector<glm::vec3>& localVerts, const std::vector<glm::ivec3>& tris, const glm::mat4& worldTr, std::vector<glm::vec3>& outVerts, std::vector<glm::vec3>& outFaceAxes, std::vector<glm::vec3>& outEdgeDirs, glm::vec3& outCenter);
};

class OBBCollider;
class AABBCollider;
class ConvexCollider;

class OBBCollider : public Collider {
public:
    OBBCollider(const glm::vec3 position, const glm::vec3 rotation, const std::string& parentName = "", const glm::vec3 halfSize = {0.5f, 0.5f, 0.5f})
        : Collider("collision_" + parentName, "", position, rotation, {1.0f, 1.0f, 1.0f}), halfSize(halfSize) {}

    ColliderType getColliderType() const override { return ColliderType::OBB; }

    ColliderAABB getWorldAABB() const override {
        glm::mat4 tr = const_cast<OBBCollider*>(this)->getWorldTransform();
        auto corners = buildOBBCorners(tr, halfSize);
        return aabbFromCorners(corners);
    }
    bool intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos, const glm::vec3& deltaRot) const override;
    glm::vec3 getHalfSize() const { return halfSize; }

private:
    glm::vec3 halfSize;
};

class AABBCollider : public Collider {
public:
    AABBCollider(const glm::vec3 position, const glm::vec3 rotation, const std::string& parentName = "", const glm::vec3 halfSize = {0.5f, 0.5f, 0.5f})
        : Collider("collision_" + parentName, "", position, rotation, {1.0f,1.0f,1.0f}), half(halfSize) {}
    ColliderType getColliderType() const override { return ColliderType::AABB; }
    ColliderAABB getWorldAABB() const override {
        glm::mat4 tr = const_cast<AABBCollider*>(this)->getWorldTransform();
        auto corners = buildOBBCorners(tr, half);
        return aabbFromCorners(corners);
    }
    bool intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos, const glm::vec3& deltaRot) const override;
private:
    glm::vec3 half;
};

class ConvexCollider : public Collider {
public:
    ConvexCollider(const glm::vec3 position, const glm::vec3 rotation, const std::string& parentName = "")
        : Collider("collision_" + parentName, "", position, rotation, {1.0f,1.0f,1.0f}) {}
    ColliderType getColliderType() const override { return ColliderType::Convex; }
    ColliderAABB getWorldAABB() const override;

    bool intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos, const glm::vec3& deltaRot) const override;

    void setVertices(const std::vector<float>& positions, const std::vector<uint32_t>& indices, const glm::vec3& rotationDegrees = glm::vec3(0.0f));

    void setVerticesInterleaved(const std::vector<float>& interleaved, size_t strideFloats, size_t positionOffsetFloats, const std::vector<uint32_t>& indices, const glm::vec3& rotationDegrees = glm::vec3(0.0f));

    const std::vector<glm::vec3>& getVertices() const { return localVertices; }
    const std::vector<glm::ivec3>& getTriangles() const { return triangles; }
    const std::vector<glm::vec3>& getWorldVerts() const { ensureCacheUpdated(); return worldVerts; }
    const std::vector<glm::vec3>& getFaceAxes() const { ensureCacheUpdated(); return faceAxesCached; }
    const std::vector<glm::vec3>& getEdgeDirs() const { ensureCacheUpdated(); return edgeDirsCached; }
    glm::vec3 getWorldCenter() const { ensureCacheUpdated(); return worldCenter; }
private:
    std::vector<glm::vec3> localVertices;
    std::vector<glm::ivec3> triangles;
    mutable std::vector<glm::vec3> worldVerts;
    mutable std::vector<glm::vec3> faceAxesCached;
    mutable std::vector<glm::vec3> edgeDirsCached;
    mutable glm::vec3 worldCenter{0.0f};
    mutable glm::mat4 lastWorldTr{0.0f};
    mutable bool cacheValid{false};

    void ensureCacheUpdated() const;
};