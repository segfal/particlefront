#include "Collider.h"

#include <iostream>

std::array<glm::vec3, 8> Collider::buildOBBCorners(const glm::mat4& transform, const glm::vec3& half) {
    std::array<glm::vec3, 8> corners{};
    static const glm::vec3 offsets[8] = {
        {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f}, {1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f}, {1.0f, -1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}
    };
    for (int i = 0; i < 8; ++i) {
        glm::vec3 local = offsets[i] * half;
        corners[i] = glm::vec3(transform * glm::vec4(local, 1.0f));
    }
    return corners;
}

void Collider::projectOntoAxis(const std::array<glm::vec3, 8>& corners, const glm::vec3& axis, float& min, float& max) {
    min = max = glm::dot(corners[0], axis);
    for (int i = 1; i < 8; ++i) {
        float projection = glm::dot(corners[i], axis);
        if (projection < min) min = projection;
        if (projection > max) max = projection;
    }
}

ColliderAABB Collider::aabbFromCorners(const std::array<glm::vec3, 8>& corners) {
    ColliderAABB box{corners[0], corners[0]};
    for (int i = 1; i < 8; ++i) {
        box.min = glm::min(box.min, corners[i]);
        box.max = glm::max(box.max, corners[i]);
    }
    return box;
}

std::array<glm::vec3,8> Collider::cornersFromAABB(const ColliderAABB& a) {
    const glm::vec3& mi = a.min; const glm::vec3& ma = a.max;
    return { glm::vec3{mi.x,mi.y,mi.z}, glm::vec3{ma.x,mi.y,mi.z}, glm::vec3{mi.x,ma.y,mi.z}, glm::vec3{ma.x,ma.y,mi.z},
             glm::vec3{mi.x,mi.y,ma.z}, glm::vec3{ma.x,mi.y,ma.z}, glm::vec3{mi.x,ma.y,ma.z}, glm::vec3{ma.x,ma.y,ma.z} };
}

bool Collider::aabbOverlapMTV(const ColliderAABB& a, const ColliderAABB& b, CollisionMTV& out) {
    glm::vec3 aCenter = 0.5f * (a.min + a.max);
    glm::vec3 bCenter = 0.5f * (b.min + b.max);
    glm::vec3 aHalf = 0.5f * (a.max - a.min);
    glm::vec3 bHalf = 0.5f * (b.max - b.min);
    glm::vec3 d = bCenter - aCenter;
    float ox = aHalf.x + bHalf.x - std::abs(d.x);
    float oy = aHalf.y + bHalf.y - std::abs(d.y);
    float oz = aHalf.z + bHalf.z - std::abs(d.z);
    if (ox <= 0 || oy <= 0 || oz <= 0) return false;
    if (ox < oy && ox < oz) {
        out.penetration = ox;
        out.normal = glm::vec3((d.x < 0 ? -1.0f : 1.0f), 0.0f, 0.0f);
    } else if (oy < oz) {
        out.penetration = oy;
        out.normal = glm::vec3(0.0f, (d.y < 0 ? -1.0f : 1.0f), 0.0f);
    } else {
        out.penetration = oz;
        out.normal = glm::vec3(0.0f, 0.0f, (d.z < 0 ? -1.0f : 1.0f));
    }
    out.mtv = out.normal * out.penetration;
    return true;
}

bool Collider::aabbIntersects(const ColliderAABB& a, const ColliderAABB& b, float margin) {
    if (a.min.x > b.max.x + margin || a.max.x < b.min.x - margin) return false;
    if (a.min.y > b.max.y + margin || a.max.y < b.min.y - margin) return false;
    if (a.min.z > b.max.z + margin || a.max.z < b.min.z - margin) return false;
    return true;
}

glm::vec3 Collider::normalizeOrZero(const glm::vec3& v) {
    float l = glm::length(v);
    return l > 1e-6f ? (v / l) : glm::vec3(0.0f);
}

void Collider::addAxisUnique(std::vector<glm::vec3>& axes, const glm::vec3& axis) {
    glm::vec3 n = normalizeOrZero(axis);
    if (glm::length(n) < 1e-6f) return;
    for (const auto& a : axes) {
        if (std::abs(glm::dot(a, n)) > 0.9999f) return;
    }
    axes.emplace_back(n);
}

void Collider::projectVertsOntoAxis(const std::vector<glm::vec3>& verts, const glm::vec3& axis, float& mn, float& mx, const glm::vec3& offset) {
    if (verts.empty()) { mn = mx = 0.0f; return; }
#if defined(USE_OPENMP)
    float mnLocal = std::numeric_limits<float>::infinity();
    float mxLocal = -std::numeric_limits<float>::infinity();
    #pragma omp parallel for reduction(min:mnLocal) reduction(max:mxLocal)
    for (int i = 0; i < static_cast<int>(verts.size()); ++i) {
        float p = glm::dot(verts[static_cast<size_t>(i)] + offset, axis);
        if (p < mnLocal) mnLocal = p;
        if (p > mxLocal) mxLocal = p;
    }
    mn = mnLocal;
    mx = mxLocal;
#else
    mn = mx = glm::dot(verts[0] + offset, axis);
    for (size_t i=1; i<verts.size(); ++i) {
        float p = glm::dot(verts[i] + offset, axis);
        if (p < mn) mn = p;
        if (p > mx) mx = p;
    }
#endif
}

bool Collider::satMTV(const std::vector<glm::vec3>& vertsA, const std::vector<glm::vec3>& faceAxesA, const std::vector<glm::vec3>& edgeDirsA, const std::vector<glm::vec3>& vertsB, const std::vector<glm::vec3>& faceAxesB, const std::vector<glm::vec3>& edgeDirsB, const glm::vec3& centerDelta, CollisionMTV& out, const glm::vec3& offsetA, const glm::vec3& offsetB) {
    constexpr float kEps = 1e-6f;
    std::vector<glm::vec3> axes;
    axes.reserve(faceAxesA.size()+faceAxesB.size()+edgeDirsA.size()*edgeDirsB.size());
    for (auto& a : faceAxesA) {
        addAxisUnique(axes, a);
    }
    for (auto& b : faceAxesB) {
        addAxisUnique(axes, b);
    }
    for (auto& ea : edgeDirsA) {
        for (auto& eb : edgeDirsB) {
            addAxisUnique(axes, glm::cross(ea, eb));
        }
    }

    float minOverlap = std::numeric_limits<float>::max();
    glm::vec3 bestAxis(0.0f);

#if defined(USE_OPENMP)
    const int m = static_cast<int>(axes.size());
    if (m == 0) return false;
    std::vector<float> overlaps(static_cast<size_t>(m));
    #pragma omp parallel for
    for (int i = 0; i < m; ++i) {
        float aMin, aMax, bMin, bMax;
        const glm::vec3& axis = axes[static_cast<size_t>(i)];
        projectVertsOntoAxis(vertsA, axis, aMin, aMax, offsetA);
        projectVertsOntoAxis(vertsB, axis, bMin, bMax, offsetB);
        overlaps[static_cast<size_t>(i)] = std::min(aMax, bMax) - std::max(aMin, bMin);
    }
    for (int i = 0; i < m; ++i) {
        if (overlaps[static_cast<size_t>(i)] <= kEps) return false;
    }
    int bestIdx = -1;
    #pragma omp parallel
    {
        float localMin = std::numeric_limits<float>::max();
        int localIdx = -1;
        #pragma omp for nowait
        for (int i = 0; i < m; ++i) {
            float ov = overlaps[static_cast<size_t>(i)];
            if (ov < localMin) { localMin = ov; localIdx = i; }
        }
        #pragma omp critical
        {
            if (localMin < minOverlap) { minOverlap = localMin; bestIdx = localIdx; }
        }
    }
    if (minOverlap <= kEps || bestIdx < 0) return false;
    bestAxis = axes[static_cast<size_t>(bestIdx)];
#else
    int axisIdx = 0;
    for (const auto& axis : axes) {
        float aMin, aMax, bMin, bMax;
        projectVertsOntoAxis(vertsA, axis, aMin, aMax, offsetA);
        projectVertsOntoAxis(vertsB, axis, bMin, bMax, offsetB);
        float overlap = std::min(aMax, bMax) - std::max(aMin, bMin);
        if (overlap <= kEps) {
            return false;
        }
        if (overlap < minOverlap) {
            minOverlap = overlap;
            bestAxis = axis;
        }
        axisIdx++;
    }
    if (minOverlap <= kEps) return false;
#endif
    if (glm::dot(bestAxis, centerDelta) < 0.0f) bestAxis = -bestAxis;
    out.normal = bestAxis; out.penetration = minOverlap; out.mtv = bestAxis * minOverlap; 
    return true;
}

void Collider::buildConvexData(const std::vector<glm::vec3>& localVerts, const std::vector<glm::ivec3>& tris, const glm::mat4& worldTr, std::vector<glm::vec3>& outVerts, std::vector<glm::vec3>& outFaceAxes, std::vector<glm::vec3>& outEdgeDirs, glm::vec3& outCenter) {
    outVerts.clear(); outFaceAxes.clear(); outEdgeDirs.clear(); outCenter = glm::vec3(0.0f);
    outVerts.resize(localVerts.size());
#if defined(USE_OPENMP)
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(localVerts.size()); ++i) {
        outVerts[i] = (glm::vec3(worldTr * glm::vec4(localVerts[i], 1.0f)));
    }
    if (outVerts.empty()) return;
    for (auto& t : tris) {
        if (static_cast<size_t>(t.x) >= outVerts.size() || 
            static_cast<size_t>(t.y) >= outVerts.size() || 
            static_cast<size_t>(t.z) >= outVerts.size()) {
            continue;
        }
        glm::vec3 a = outVerts[static_cast<size_t>(t.x)];
        glm::vec3 b = outVerts[static_cast<size_t>(t.y)];
        glm::vec3 c = outVerts[static_cast<size_t>(t.z)];
        glm::vec3 normal = glm::cross(b - a, c - a);
        float len = glm::length(normal);
        if (len > 1e-6f) {
            addAxisUnique(outFaceAxes, normal / len);
        }
        addAxisUnique(outEdgeDirs, normalizeOrZero(b-a));
        addAxisUnique(outEdgeDirs, normalizeOrZero(c-b));
        addAxisUnique(outEdgeDirs, normalizeOrZero(a-c));
    }
    if (outFaceAxes.empty()) {
        outFaceAxes = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
    }
    if (outEdgeDirs.empty()) outEdgeDirs = outFaceAxes;
    float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
#if defined(USE_OPENMP)
    #pragma omp parallel for reduction(+:centerX, centerY, centerZ)
#endif
    for (int i = 0; i < static_cast<int>(outVerts.size()); ++i) {
        const auto& v = outVerts[i];
        centerX += v.x; centerY += v.y; centerZ += v.z;
    }
    outCenter = glm::vec3(centerX, centerY, centerZ) / static_cast<float>(outVerts.size());
}

ColliderAABB ConvexCollider::getWorldAABB() const {
    ensureCacheUpdated();
    if (worldVerts.empty()) {
        glm::vec3 p = glm::vec3(computeWorldTransform(const_cast<ConvexCollider*>(this))[3]);
        return {p - glm::vec3(0.001f), p + glm::vec3(0.001f)};
    }
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();
#if defined(USE_OPENMP)
    #pragma omp parallel for reduction(min:minX, minY, minZ) reduction(max:maxX, maxY, maxZ)
    for (int i = 0; i < static_cast<int>(worldVerts.size()); ++i) {
        const glm::vec3& w = worldVerts[static_cast<size_t>(i)];
        if (w.x < minX) minX = w.x; if (w.y < minY) minY = w.y; if (w.z < minZ) minZ = w.z;
        if (w.x > maxX) maxX = w.x; if (w.y > maxY) maxY = w.y; if (w.z > maxZ) maxZ = w.z;
    }
#else
    for (const auto& w : worldVerts) {
        minX = glm::min(minX, w.x);
        maxX = glm::max(maxX, w.x);
        minY = glm::min(minY, w.y);
        maxY = glm::max(maxY, w.y);
        minZ = glm::min(minZ, w.z);
        maxZ = glm::max(maxZ, w.z);
    }
#endif
    return ColliderAABB{ glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ) };
}

bool ConvexCollider::intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos, const glm::vec3& deltaRot) const {
    (void)deltaRot;
    ColliderAABB aabbThis = getWorldAABB();
    if (glm::length(deltaPos) > 0.0f) {
        aabbThis.min += deltaPos;
        aabbThis.max += deltaPos;
    }
    ColliderAABB aabbOther = other.getWorldAABB();
    if (!Collider::aabbIntersects(aabbThis, aabbOther, 0.001f)) return false;
    ensureCacheUpdated();
    const std::vector<glm::vec3>& vertsA = worldVerts;
    const std::vector<glm::vec3>& faceAxesA = faceAxesCached;
    const std::vector<glm::vec3>& edgesA = edgeDirsCached;
    glm::vec3 centerA = worldCenter + deltaPos;
    glm::mat4 otherTr = computeWorldTransform(const_cast<Collider*>(&other));
    std::vector<glm::vec3> vertsB; std::vector<glm::vec3> faceAxesB; std::vector<glm::vec3> edgesB; glm::vec3 centerB(0.0f);
    if (other.getColliderType() == ColliderType::OBB) {
        auto cornersB = Collider::buildOBBCorners(otherTr, static_cast<const OBBCollider&>(other).getHalfSize());
        vertsB.assign(cornersB.begin(), cornersB.end());
        faceAxesB = { Collider::normalizeOrZero(glm::vec3(otherTr[0])), Collider::normalizeOrZero(glm::vec3(otherTr[1])), Collider::normalizeOrZero(glm::vec3(otherTr[2])) };
        edgesB = faceAxesB; centerB = glm::vec3(otherTr[3]);
    } else if (other.getColliderType() == ColliderType::AABB) {
        ColliderAABB b = other.getWorldAABB(); auto corners = Collider::cornersFromAABB(b);
        vertsB.assign(corners.begin(), corners.end());
        faceAxesB = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
        edgesB = faceAxesB; centerB = 0.5f * (b.min + b.max);
    } else {
        const auto& cvx = static_cast<const ConvexCollider&>(other);
        const auto& oVerts = cvx.getWorldVerts();
        if (!oVerts.empty()) {
            vertsB = oVerts;
            faceAxesB = cvx.getFaceAxes();
            edgesB = cvx.getEdgeDirs();
            centerB = cvx.getWorldCenter();
        } else {
            ColliderAABB b = other.getWorldAABB(); auto corners = Collider::cornersFromAABB(b);
            vertsB.assign(corners.begin(), corners.end());
            faceAxesB = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
            edgesB = faceAxesB; centerB = 0.5f*(b.min+b.max);
        }
    }
    return Collider::satMTV(vertsA, faceAxesA, edgesA, vertsB, faceAxesB, edgesB, centerA - centerB, out, deltaPos);
}

void ConvexCollider::setVertices(const std::vector<float>& positions, const std::vector<uint32_t>& indices, const glm::vec3& rotationDegrees) {
    localVertices.clear();
    const size_t vcount = positions.size() / 3;
    localVertices.resize(vcount);
    glm::mat4 rotMat(1.0f);
    if (glm::length(rotationDegrees) > 0.001f) {
        rotMat = glm::rotate(rotMat, glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotMat = glm::rotate(rotMat, glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotMat = glm::rotate(rotMat, glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
    }

#if defined(USE_OPENMP)
    #pragma omp parallel for
#endif
    for (int i = 0; i < static_cast<int>(vcount); ++i) {
        glm::vec3 v(positions[i*3 + 0], positions[i*3 + 1], positions[i*3 + 2]);
        if (glm::length(rotationDegrees) > 0.001f) {
            v = glm::vec3(rotMat * glm::vec4(v, 1.0f));
        }
        localVertices[i] = v;
    }

    triangles.clear();
    const size_t tcount = indices.size() / 3;
    triangles.resize(tcount);
#if defined(USE_OPENMP)
    #pragma omp parallel for
#endif
    for (int t = 0; t < static_cast<int>(tcount); ++t) {
        uint32_t i0 = indices[t*3 + 0];
        uint32_t i1 = indices[t*3 + 1];
        uint32_t i2 = indices[t*3 + 2];
        if (i0 < vcount && i1 < vcount && i2 < vcount) {
            triangles[t] = glm::ivec3(static_cast<int>(i0), static_cast<int>(i1), static_cast<int>(i2));
        }
    }
    cacheValid = false;
}

void ConvexCollider::setVerticesInterleaved(const std::vector<float>& interleaved, size_t strideFloats, size_t positionOffsetFloats, const std::vector<uint32_t>& indices, const glm::vec3& rotationDegrees) {
    localVertices.clear();
    if (strideFloats < positionOffsetFloats + 3) {
        std::cerr << "ConvexCollider::setVerticesInterleaved - invalid stride/offset (" << strideFloats << ", " << positionOffsetFloats << ")\n";
        return;
    }
    const size_t vcount = interleaved.size() / strideFloats;
    localVertices.reserve(vcount);
    glm::mat4 rotMat(1.0f);
    bool applyRot = glm::length(rotationDegrees) > 0.001f;
    if (applyRot) {
        rotMat = glm::rotate(rotMat, glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotMat = glm::rotate(rotMat, glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotMat = glm::rotate(rotMat, glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    for (size_t i = 0; i < vcount; ++i) {
        size_t base = i * strideFloats + positionOffsetFloats;
        if (base + 2 >= interleaved.size()) break;
        glm::vec3 v(interleaved[base + 0], interleaved[base + 1], interleaved[base + 2]);
        if (applyRot) v = glm::vec3(rotMat * glm::vec4(v, 1.0f));
        localVertices.emplace_back(v);
    }

    triangles.clear();
    const size_t tcount = indices.size() / 3;
    triangles.reserve(tcount);
    for (size_t t = 0; t < tcount; ++t) {
        uint32_t i0 = indices[t*3 + 0];
        uint32_t i1 = indices[t*3 + 1];
        uint32_t i2 = indices[t*3 + 2];
        if (i0 < vcount && i1 < vcount && i2 < vcount) {
            triangles.emplace_back(static_cast<int>(i0), static_cast<int>(i1), static_cast<int>(i2));
        }
    }
    cacheValid = false;
}

void ConvexCollider::ensureCacheUpdated() const {
    glm::mat4 tr = computeWorldTransform(const_cast<ConvexCollider*>(this));
    bool same = cacheValid;
    if (same) {
        for (int c=0;c<4 && same;++c) {
            for (int r=0;r<4;++r) {
                if (std::abs(tr[c][r] - lastWorldTr[c][r]) > 1e-6f) {
                    same = false;
                    break;
                }
            }
        }
    }
    if (!same) {
        worldVerts.clear(); faceAxesCached.clear(); edgeDirsCached.clear(); worldCenter = glm::vec3(0.0f);
        worldVerts.resize(localVertices.size());
#if defined(USE_OPENMP)
        #pragma omp parallel for
#endif
        for (int i = 0; i < static_cast<int>(localVertices.size()); ++i) {
            worldVerts[i] = glm::vec3(tr * glm::vec4(localVertices[i], 1.0f));
        }
        
        if (!worldVerts.empty()) {
            for (const auto& t : triangles) {
                if (static_cast<size_t>(t.x) >= worldVerts.size() || 
                    static_cast<size_t>(t.y) >= worldVerts.size() || 
                    static_cast<size_t>(t.z) >= worldVerts.size()) {
                    continue;
                }
                glm::vec3 a = worldVerts[static_cast<size_t>(t.x)];
                glm::vec3 b = worldVerts[static_cast<size_t>(t.y)];
                glm::vec3 c = worldVerts[static_cast<size_t>(t.z)];
                glm::vec3 normal = glm::cross(b - a, c - a);
                float len = glm::length(normal);
                if (len > 1e-6f) {
                    Collider::addAxisUnique(faceAxesCached, normal / len);
                }
                Collider::addAxisUnique(edgeDirsCached, Collider::normalizeOrZero(b-a));
                Collider::addAxisUnique(edgeDirsCached, Collider::normalizeOrZero(c-b));
                Collider::addAxisUnique(edgeDirsCached, Collider::normalizeOrZero(a-c));
            }
            if (faceAxesCached.empty()) {
                faceAxesCached = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
            }
            if (edgeDirsCached.empty()) edgeDirsCached = faceAxesCached;
            
            float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
#if defined(USE_OPENMP)
            #pragma omp parallel for reduction(+:centerX, centerY, centerZ)
#endif
            for (int i = 0; i < static_cast<int>(worldVerts.size()); ++i) {
                const auto& v = worldVerts[static_cast<size_t>(i)];
                centerX += v.x; centerY += v.y; centerZ += v.z;
            }
            worldCenter = glm::vec3(centerX, centerY, centerZ) / static_cast<float>(worldVerts.size());
        }
        lastWorldTr = tr;
        cacheValid = true;
    }
}

bool OBBCollider::intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos, const glm::vec3& deltaRot) const {
    glm::mat4 baseThis = computeWorldTransform(const_cast<OBBCollider*>(this));
    glm::mat4 thisTransform = baseThis;
    thisTransform[3] += glm::vec4(deltaPos, 0.0f);
    if (glm::length(deltaRot) > 0.0f) {
        glm::mat4 r(1.0f);
        r = glm::rotate(r, glm::radians(deltaRot.x), glm::vec3(1,0,0));
        r = glm::rotate(r, glm::radians(deltaRot.y), glm::vec3(0,1,0));
        r = glm::rotate(r, glm::radians(deltaRot.z), glm::vec3(0,0,1));
        thisTransform *= r;
    }

    auto cornersA = Collider::buildOBBCorners(thisTransform, halfSize);
    ColliderAABB aabbA = Collider::aabbFromCorners(cornersA);
    ColliderAABB aabbB = other.getWorldAABB();
    if (!Collider::aabbIntersects(aabbA, aabbB, 0.001f)) return false;
    std::vector<glm::vec3> vertsA(cornersA.begin(), cornersA.end());
    std::vector<glm::vec3> faceAxesA = { Collider::normalizeOrZero(glm::vec3(thisTransform[0])), Collider::normalizeOrZero(glm::vec3(thisTransform[1])), Collider::normalizeOrZero(glm::vec3(thisTransform[2])) };
    std::vector<glm::vec3> edgesA = faceAxesA;
    glm::vec3 centerA = glm::vec3(thisTransform[3]);

    glm::mat4 otherTr = computeWorldTransform(const_cast<Collider*>(&other));
    std::vector<glm::vec3> vertsB; std::vector<glm::vec3> faceAxesB; std::vector<glm::vec3> edgesB; glm::vec3 centerB(0.0f);
    if (other.getColliderType() == ColliderType::OBB) {
        auto cornersB = Collider::buildOBBCorners(otherTr, static_cast<const OBBCollider&>(other).getHalfSize());
        vertsB.assign(cornersB.begin(), cornersB.end());
        faceAxesB = { Collider::normalizeOrZero(glm::vec3(otherTr[0])), Collider::normalizeOrZero(glm::vec3(otherTr[1])), Collider::normalizeOrZero(glm::vec3(otherTr[2])) };
        edgesB = faceAxesB;
        centerB = glm::vec3(otherTr[3]);
    } else if (other.getColliderType() == ColliderType::AABB) {
        ColliderAABB box = other.getWorldAABB();
        auto corners = Collider::cornersFromAABB(box);
        vertsB.assign(corners.begin(), corners.end());
        faceAxesB = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
        edgesB = faceAxesB;
        centerB = 0.5f * (box.min + box.max);
    } else {
        const auto& cvx = static_cast<const ConvexCollider&>(other);
        const auto& oVerts = cvx.getWorldVerts();
        if (!oVerts.empty()) {
            vertsB = oVerts;
            faceAxesB = cvx.getFaceAxes();
            edgesB = cvx.getEdgeDirs();
            centerB = cvx.getWorldCenter();
        } else {
            ColliderAABB box = other.getWorldAABB();
            auto corners = Collider::cornersFromAABB(box);
            vertsB.assign(corners.begin(), corners.end());
            faceAxesB = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
            edgesB = faceAxesB;
            centerB = 0.5f * (box.min + box.max);
        }
    }
    return Collider::satMTV(vertsA, faceAxesA, edgesA, vertsB, faceAxesB, edgesB, centerA - centerB, out);
}

bool AABBCollider::intersectsMTV(const Collider& other, CollisionMTV& out, const glm::vec3& deltaPos, const glm::vec3& deltaRot) const {
    (void)deltaRot;
    glm::mat4 tr = computeWorldTransform(const_cast<AABBCollider*>(this));
    tr[3] += glm::vec4(deltaPos, 0.0f);
    if (other.getColliderType() == ColliderType::AABB) {
        ColliderAABB a = Collider::aabbFromCorners(Collider::buildOBBCorners(tr, half));
        ColliderAABB b = other.getWorldAABB();
        return Collider::aabbOverlapMTV(a,b,out);
    }
    auto cornersA = Collider::buildOBBCorners(tr, half);
    ColliderAABB aabbA = Collider::aabbFromCorners(cornersA);
    ColliderAABB aabbB = other.getWorldAABB();
    if (!Collider::aabbIntersects(aabbA, aabbB, 0.001f)) return false;

    std::vector<glm::vec3> vertsA(cornersA.begin(), cornersA.end());
    std::vector<glm::vec3> faceAxesA = { Collider::normalizeOrZero(glm::vec3(tr[0])), Collider::normalizeOrZero(glm::vec3(tr[1])), Collider::normalizeOrZero(glm::vec3(tr[2])) };
    std::vector<glm::vec3> edgesA = faceAxesA;
    glm::vec3 centerA = glm::vec3(tr[3]);

    glm::mat4 otherTr = computeWorldTransform(const_cast<Collider*>(&other));
    std::vector<glm::vec3> vertsB; std::vector<glm::vec3> faceAxesB; std::vector<glm::vec3> edgesB; glm::vec3 centerB(0.0f);
    if (other.getColliderType() == ColliderType::OBB) {
        auto cornersB = Collider::buildOBBCorners(otherTr, static_cast<const OBBCollider&>(other).getHalfSize());
        vertsB.assign(cornersB.begin(), cornersB.end());
        faceAxesB = { Collider::normalizeOrZero(glm::vec3(otherTr[0])), Collider::normalizeOrZero(glm::vec3(otherTr[1])), Collider::normalizeOrZero(glm::vec3(otherTr[2])) };
        edgesB = faceAxesB;
        centerB = glm::vec3(otherTr[3]);
    } else { // Convex
        const auto& cvx = static_cast<const ConvexCollider&>(other);
        const auto& oVerts = cvx.getWorldVerts();
        if (!oVerts.empty()) {
            vertsB = oVerts;
            faceAxesB = cvx.getFaceAxes();
            edgesB = cvx.getEdgeDirs();
            centerB = cvx.getWorldCenter();
        } else {
            ColliderAABB b = other.getWorldAABB();
            auto corners = Collider::cornersFromAABB(b);
            vertsB.assign(corners.begin(), corners.end());
            faceAxesB = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
            edgesB = faceAxesB; centerB = 0.5f * (b.min + b.max);
        }
    }
    return Collider::satMTV(vertsA, faceAxesA, edgesA, vertsB, faceAxesB, edgesB, centerA - centerB, out);
}
