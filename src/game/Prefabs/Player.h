#pragma once
#include <CharacterEntity.h>
#include <InputManager.h>
#include <Camera.h>
#include <Collider.h>
#include <Renderer.h>
#include <UIManager.h>
#include <UIObject.h>
#include <TextObject.h>
#include <EntityManager.h>
#include <iostream>

class Player : public CharacterEntity {
private:
    Camera* playerCamera = nullptr;

    float health = 100.0f;
    float maxHealth = 100.0f;

public:
    Player(const glm::vec3& position = {0.0f, 0.0f, 0.0f}, const glm::vec3& rotation = {0.0f, 0.0f, 0.0f})
        : CharacterEntity("player", "gbuffer", position, rotation) {
        playerCamera = new Camera({0.0f, 1.6f, 0.0f}, {0.0f, rotation.y, 0.0f}, 70.0f);
        this->addChild(playerCamera);
        OBBCollider* box = new OBBCollider({0.0f, 0.6f, 0.0f}, {0.0f, 0.0f, 0.0f}, this->getName(), {0.5f, 1.8f, 0.5f});
        this->addChild(box);
        Renderer::getInstance()->setActiveCamera(playerCamera);
        InputManager::getInstance()->registerListener([this](const std::vector<InputEvent>& events) { this->registerInput(events); });
    }

    void registerInput(const std::vector<InputEvent>& events) {
        for (const auto& event : events) {
            if (event.type == InputEvent::Type::KeyPress) {
                switch (event.keyEvent.key) {
                    case GLFW_KEY_W:
                        move({0.0f, 0.0f, -1.0f});
                        break;
                    case GLFW_KEY_S:
                        move({0.0f, 0.0f, 1.0f});
                        break;
                    case GLFW_KEY_A:
                        move({-1.0f, 0.0f, 0.0f});
                        break;
                    case GLFW_KEY_D:
                        move({1.0f, 0.0f, 0.0f});
                        break;
                    case GLFW_KEY_SPACE:
                        jump();
                        break;
                    default:
                        break;
                }
            }
            else if (event.type == InputEvent::Type::KeyRelease) {
                switch (event.keyEvent.key) {
                    case GLFW_KEY_W:
                        stopMove({0.0f, 0.0f, -1.0f});
                        break;
                    case GLFW_KEY_S:
                        stopMove({0.0f, 0.0f, 1.0f});
                        break;
                    case GLFW_KEY_A:
                        stopMove({-1.0f, 0.0f, 0.0f});
                        break;
                    case GLFW_KEY_D:
                        stopMove({1.0f, 0.0f, 0.0f});
                        break;
                    default:
                        break;
                }
            }
            else if (event.type == InputEvent::Type::MouseMove) {
                if (!Renderer::getInstance()->isCursorLocked()) {
                    return;
                }
                constexpr float sensitivity = 0.05f;
                rotate({0.0f, static_cast<float>(-event.mouseMoveEvent.x) * sensitivity, static_cast<float>(-event.mouseMoveEvent.y) * sensitivity});
            }
        }
    }

    void registerHit(float damage) {
        health -= damage;
        if (health < 0.0f) health = 0.0f;
        std::cout << "[Player] Took " << damage << " damage. Health: " << health << "/" << maxHealth << std::endl;
    }

    void update(float deltaTime) override {
        static int frameCount = 0;
        frameCount++;

        // Log player position every 60 frames
        if (frameCount % 60 == 0) {
            glm::vec3 myPos = getPosition();

            // Get enemy position if it exists
            Entity* enemyEntity = EntityManager::getInstance()->getEntity("enemy");
            if (enemyEntity) {
                glm::vec3 enemyPos = enemyEntity->getPosition();
                glm::vec3 toEnemy = enemyPos - myPos;
                glm::vec3 toEnemyXZ = toEnemy;
                toEnemyXZ.y = 0.0f;
                float distanceXZ = glm::length(toEnemyXZ);
        }

        // Call parent update
        CharacterEntity::update(deltaTime);
    }

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
};