#pragma once
#include "Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <limits>
#include <vector>
#include <cmath>
#include "../utils.h"

class CollisionBox : public Entity {
public:
    CollisionBox(const glm::vec3 position, const glm::vec3 rotation, const std::string& parentName = "", const glm::vec3 halfSize = {0.5f, 0.5f, 0.5f})
        : Entity("collision_" + parentName, "", position, rotation, {1.0f, 1.0f, 1.0f}), halfSize(halfSize) {}

    glm::vec3 intersects(const CollisionBox* other, const glm::vec3& deltaPos, const glm::vec3& deltaRot = glm::vec3(0.0f)) const {
        glm::mat4 baseThis = computeWorldTransform(const_cast<CollisionBox*>(this));
        glm::mat4 thisTransform = baseThis;
        thisTransform[3] += glm::vec4(deltaPos, 0.0f);
        if (glm::length(deltaRot) > 0.0f) {
            glm::mat4 rotationDelta(1.0f);
            rotationDelta = glm::rotate(rotationDelta, glm::radians(deltaRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
            rotationDelta = glm::rotate(rotationDelta, glm::radians(deltaRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rotationDelta = glm::rotate(rotationDelta, glm::radians(deltaRot.z), glm::vec3(0.0f, 0.0f, 1.0f));
            thisTransform *= rotationDelta;
        }
        glm::mat4 otherTransform = computeWorldTransform(const_cast<CollisionBox*>(other));
        auto buildCorners = [](const glm::mat4& transform, const glm::vec3& half){
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
        };
        auto thisCorners = buildCorners(thisTransform, halfSize);
        auto otherCorners = buildCorners(otherTransform, other->halfSize);
        glm::vec3 thisAxesRaw[3] = {
            glm::vec3(thisTransform[0]),
            glm::vec3(thisTransform[1]),
            glm::vec3(thisTransform[2])
        };
        glm::vec3 otherAxesRaw[3] = {
            glm::vec3(otherTransform[0]),
            glm::vec3(otherTransform[1]),
            glm::vec3(otherTransform[2])
        };
        glm::vec3 thisAxes[3];
        glm::vec3 otherAxes[3];
        for (int i = 0; i < 3; ++i) {
            float lenThis = glm::length(thisAxesRaw[i]);
            float lenOther = glm::length(otherAxesRaw[i]);
            thisAxes[i] = lenThis > 1e-5f ? thisAxesRaw[i] / lenThis : glm::vec3(0.0f);
            otherAxes[i] = lenOther > 1e-5f ? otherAxesRaw[i] / lenOther : glm::vec3(0.0f);
        }
        struct AxisCandidate {
            glm::vec3 axis;
            int priority;
        };
        std::vector<AxisCandidate> axes;
        axes.reserve(15);
        auto tryAddAxis = [&axes](const glm::vec3& axis, int priority) {
            float len = glm::length(axis);
            if (len < 1e-5f) {
                return;
            }
            axes.push_back({axis / len, priority});
        };
        for (const auto& axis : thisAxes) {
            tryAddAxis(axis, 0);
        }
        for (const auto& axis : otherAxes) {
            tryAddAxis(axis, 1);
        }
        for (const auto& a : thisAxes) {
            for (const auto& b : otherAxes) {
                tryAddAxis(glm::cross(a, b), 2);
            }
        }
        glm::vec3 bestAxis(0.0f);
        float minOverlap = std::numeric_limits<float>::max();
        bool hasAxis = false;
        int bestPriority = 3;
        constexpr float overlapEpsilon = 1e-4f;
        for (const auto& candidate : axes) {
            const glm::vec3& axis = candidate.axis;
            float aMin, aMax, bMin, bMax;
            projectOntoAxis(thisCorners, axis, aMin, aMax);
            projectOntoAxis(otherCorners, axis, bMin, bMax);
            float overlap = std::min(aMax, bMax) - std::max(aMin, bMin);
            if (overlap <= 0.0f) {
                return glm::vec3(0.0f);
            }
            if (overlap + overlapEpsilon < minOverlap || (std::abs(overlap - minOverlap) <= overlapEpsilon && candidate.priority < bestPriority)) {
                minOverlap = overlap;
                bestAxis = axis;
                hasAxis = true;
                bestPriority = candidate.priority;
            }
        }
        if (!hasAxis) {
            return glm::vec3(0.0f);
        }
        glm::vec3 centerDelta = glm::vec3(thisTransform[3]) - glm::vec3(otherTransform[3]);
        if (glm::dot(bestAxis, centerDelta) < 0.0f) {
            bestAxis = -bestAxis;
        }
        return bestAxis * minOverlap;
    }
private:
    glm::vec3 halfSize;

    void projectOntoAxis(const std::array<glm::vec3, 8>& corners, const glm::vec3& axis, float& min, float& max) const {
        min = max = glm::dot(corners[0], axis);
        for (int i = 1; i < 8; ++i) {
            float projection = glm::dot(corners[i], axis);
            if (projection < min) {
                min = projection;
            }
            if (projection > max) {
                max = projection;
            }
        }
    }
};