#pragma once
#include <algorithm>
#include <cmath>
#include "Entity.h"
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
        if (glm::length(pressed) > 0.001f) {
            glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), glm::radians(getRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 worldDirection = glm::vec3(yawRotation * glm::vec4(pressed, 0.0f));
            worldDirection.y = 0.0f;
            const float magnitude = glm::length(worldDirection);
            if (magnitude > 0.001f) {
                worldDirection = worldDirection / magnitude;
                velocity = worldDirection * moveSpeed;
                glm::vec3 newPosition = getPosition() + velocity * deltaTime;
                setPosition(newPosition);
                return;
            }
        }
        resetVelocity();
    }
    void move (const glm::vec3& delta) {
        pressed += delta;
    }
    void stopMove(const glm::vec3& delta) {
        pressed -= delta;
        if (glm::length(pressed) < 0.001f) {
            resetVelocity();
        }
    }
    void resetVelocity() {
        velocity = glm::vec3(0.0f);
        pressed = glm::vec3(0.0f);
    }
    void rotate (const glm::vec3& delta) {
        if (delta.y != 0.0f) {
            glm::vec3 bodyRot = getRotation();
            bodyRot.y = std::fmod(bodyRot.y + delta.y, 360.0f);
            if (bodyRot.y < 0.0f) bodyRot.y += 360.0f;
            setRotation(bodyRot);
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
};