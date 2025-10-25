#pragma once
#include <algorithm>
#include <cmath>
#include "Entity.h"
#include "CollisionBox.h"
#include <glm/gtc/matrix_transform.hpp>

class CharacterEntity : public Entity {
public:
    CharacterEntity(
        const std::string& name,
        const std::string& shader,
        const glm::vec3& position = {0.0f, 0.0f, 0.0f},
        const glm::vec3& rotation = {0.0f, 0.0f, 0.0f}
    ) : Entity(name, shader, position, rotation) {}
    void update(float deltaTime) override {
        // Clamp deltaTime to avoid giant steps (e.g., when resizing window)
        const float MAX_DELTA_TIME = 0.05f; // 50 ms
        deltaTime = std::min(deltaTime, MAX_DELTA_TIME);

        glm::vec3 desiredVel(0.0f);
        if (glm::length(pressed) > 0.001f) {
            glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), glm::radians(getRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 worldDir = glm::vec3(yawRotation * glm::vec4(pressed, 0.0f));
            worldDir.y = 0.0f;
            float m = glm::length(worldDir);
            if (m > 0.001f) {
                worldDir /= m;
                desiredVel = worldDir * moveSpeed;
            }
        }
        velocity.x = desiredVel.x;
        velocity.z = desiredVel.z;
        velocity.y -= 9.81f * deltaTime;

        // Substep integration to prevent tunneling through thin objects
        const float MAX_STEP_DIST = 0.25f;
        const float totalMoveLen = glm::length(velocity * deltaTime);
        int steps = totalMoveLen > 0.0f ? static_cast<int>(std::ceil(totalMoveLen / MAX_STEP_DIST)) : 1;
        steps = std::clamp(steps, 1, 8);
        const float subDt = deltaTime / static_cast<float>(steps);

        constexpr float kSkin = 0.002f; // small separation to avoid re-colliding due to numerical error
        for (int i = 0; i < steps; ++i) {
            glm::vec3 stepDelta = velocity * subDt;
            collision pred = willCollide(stepDelta);
            if (!pred.box) {
                setPosition(getPosition() + stepDelta);
            } else {
                setPosition(getPosition() + stepDelta + pred.mtv);
                float len = glm::length(pred.mtv);
                if (len > 1e-5f) {
                    glm::vec3 n = pred.mtv / len;
                    float vn = glm::dot(velocity, n);
                    if (vn < 0.0f) velocity -= vn * n;
                    // Skin offset to keep a tiny separation
                    setPosition(getPosition() + n * kSkin);
                    // If colliding with floor, stop downward velocity
                    if (n.y > 0.5f && velocity.y < 0.0f) velocity.y = 0.0f;
                }
            }
            collision post = willCollide(glm::vec3(0.0f));
            if (post.box) {
                setPosition(getPosition() + post.mtv);
                float len = glm::length(post.mtv);
                if (len > 1e-5f) {
                    glm::vec3 n = post.mtv / len;
                    float vn = glm::dot(velocity, n);
                    if (vn < 0.0f) velocity -= vn * n;
                    setPosition(getPosition() + n * kSkin);
                    if (n.y > 0.5f && velocity.y < 0.0f) velocity.y = 0.0f;
                }
            }
        }
        if (glm::length(velocity) < 1e-4f) {
            resetVelocity();
        }
    }
    void move(const glm::vec3& delta) {
        pressed += delta;
    }
    void stopMove(const glm::vec3& delta) {
        pressed -= delta;
        if (glm::length(pressed) < 0.001f) {
            resetVelocity();
        }
    }
    void jump() {
        if (std::abs(velocity.y) < 0.01f) {
            velocity.y = 10.0f;
        }
    }
    void resetVelocity() {
        velocity = glm::vec3(0.0f);
    }
    void rotate(const glm::vec3& delta) {
        if (delta.y != 0.0f) {
            glm::vec3 bodyRot = getRotation();
            bodyRot.y = std::fmod(bodyRot.y + delta.y, 360.0f);
            if (bodyRot.y < 0.0f) bodyRot.y += 360.0f;
            collision rotCol = willCollide(glm::vec3(0.0f), bodyRot - getRotation());
            if (!rotCol.box || glm::length(rotCol.mtv) < 5e-3f) {
                setRotation(bodyRot);
            }
        }

        if (delta.z != 0.0f) {
            Entity* head = this->getChild("camera");
            if (!head) {
                head = this->getChild("head");
            }
            if (head) {
                glm::vec3 headRot = head->getRotation();
                headRot.x = std::clamp(headRot.x + delta.z, -89.0f, 89.0f);
                head->setRotation(headRot);
            }
        }
    }
private:
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 pressed = glm::vec3(0.0f);
    const float moveSpeed = 10.0f;

    struct collision {
        CollisionBox* box = nullptr;
        glm::vec3 mtv = glm::vec3(0.0f);
    };

    collision willCollide(const glm::vec3& deltaPos, const glm::vec3& deltaRot = glm::vec3(0.0f)) {
        CollisionBox* myBox = nullptr;
        for (auto& child : this->getChildren()) {
            myBox = dynamic_cast<CollisionBox*>(child);
            if (myBox) break;
        }
        if (!myBox) {
            return {nullptr, glm::vec3(0.0f)};
        }
        glm::vec3 collision = glm::vec3(0.0f);
        EntityManager* entityMgr = EntityManager::getInstance();
        for (const auto& [otherName, otherEntity] : entityMgr->getAllEntities()) {
            if (otherEntity == this) continue;
            CollisionBox* otherBox = nullptr;
            for (auto& child : otherEntity->getChildren()) {
                otherBox = dynamic_cast<CollisionBox*>(child);
                if (otherBox) break;
            }
            if (!otherBox) continue;
            collision = myBox->intersects(otherBox, deltaPos, deltaRot);
            if (glm::length(collision) > 0.0f) {
                return {otherBox, collision};
            }
        }
        return {nullptr, glm::vec3(0.0f)};
    }
};