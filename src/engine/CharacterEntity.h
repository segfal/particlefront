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
        // FIX: Clamp delta time to prevent physics explosions during frame stalls (e.g., window resize)
        // Without this, a 2-second frame stall causes velocity.y -= 9.81 * 2.0 = -19.62, teleporting player through floor
        const float MAX_DELTA_TIME = 0.05f;  // 50ms max = 20 FPS minimum
        deltaTime = std::min(deltaTime, MAX_DELTA_TIME);

        bool changedPos = false;
        bool appliedCorrection = false;
        float previousVerticalVelocity = velocity.y;
        if (glm::length(pressed) > 0.001f) {
            glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), glm::radians(getRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 worldDirection = glm::vec3(yawRotation * glm::vec4(pressed, 0.0f));
            worldDirection.y = 0.0f;
            const float magnitude = glm::length(worldDirection);
            if (magnitude > 0.001f) {
                worldDirection = worldDirection / magnitude;
                glm::vec3 tempVelocity = worldDirection * moveSpeed;
                glm::vec3 newPosition = getPosition() + tempVelocity * deltaTime;
                collision moveCollision = willCollide(tempVelocity * deltaTime);
                if (!moveCollision.box) {
                    changedPos = true;
                    velocity.x = tempVelocity.x;
                    velocity.z = tempVelocity.z;
                } else {
                    if (glm::length(moveCollision.mtv) > 0.0f) {
                        glm::vec3 normal = glm::normalize(moveCollision.mtv);
                        glm::vec3 tangential = tempVelocity - glm::dot(tempVelocity, normal) * normal;
                        velocity.x = tangential.x;
                        velocity.z = tangential.z;
                    } else {
                        velocity.x = 0.0f;
                        velocity.z = 0.0f;
                    }
                }
            }
        }
        velocity.y = previousVerticalVelocity;
        velocity.y -= 9.81f * deltaTime;
        glm::vec3 newPosition = getPosition() + velocity * deltaTime;
        collision moveResult = willCollide(velocity * deltaTime);
        if (!moveResult.box) {
            setPosition(newPosition);
            changedPos = true;
        } else if (changedPos) {
            velocity.y = previousVerticalVelocity;
            newPosition = getPosition() + velocity * deltaTime;
            setPosition(newPosition);
        }
        collision col = willCollide(glm::vec3(0.0f));
        if (col.box != nullptr) {
            glm::vec3 mtv = col.mtv;
            setPosition(getPosition() + mtv);
            float mtvLength = glm::length(mtv);
            if (mtvLength > 1e-5f) {
                glm::vec3 normal = mtv / mtvLength;
                velocity -= glm::dot(velocity, normal) * normal;
            }
            changedPos = true;
            appliedCorrection = true;
        }
        if ((!changedPos && !appliedCorrection) || glm::length(velocity) < 1e-4f) {
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
            if (!willCollide(glm::vec3(0.0f), bodyRot - getRotation()).box) {
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