#pragma once
#include <CharacterEntity.h>
#include <EntityManager.h>
#include <Collider.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

class Player;

class DetectorAABB : public AABBCollider {
public:
    DetectorAABB(const glm::vec3& position, const glm::vec3& rotation, const std::string& parentName = "", const glm::vec3& halfSize = {0.5f, 0.5f, 0.5f})
        : AABBCollider(position, rotation, parentName + "_detector", halfSize) {}

    bool isEntityInside(Entity* entity) const {
        if (!entity) return false;

        ColliderAABB myAABB = getWorldAABB();

        glm::vec3 entityPos = entity->getPosition();

        return (entityPos.x >= myAABB.min.x && entityPos.x <= myAABB.max.x &&
                entityPos.y >= myAABB.min.y && entityPos.y <= myAABB.max.y &&
                entityPos.z >= myAABB.min.z && entityPos.z <= myAABB.max.z);
    }

    bool detectsEntity(Entity* entity) const {
        if (!entity) return false;

        ColliderAABB myAABB = getWorldAABB();

        for (auto* child : entity->getChildren()) {
            Collider* collider = dynamic_cast<Collider*>(child);
            if (collider) {
                ColliderAABB otherAABB = collider->getWorldAABB();

                if (!(myAABB.max.x < otherAABB.min.x || myAABB.min.x > otherAABB.max.x ||
                      myAABB.max.y < otherAABB.min.y || myAABB.min.y > otherAABB.max.y ||
                      myAABB.max.z < otherAABB.min.z || myAABB.min.z > otherAABB.max.z)) {
                    return true;
                }
            }
        }

        return false;
    }
};

class Enemy : public CharacterEntity {
private:
    DetectorAABB* detectorAABB = nullptr;
    glm::vec3 lastMoveDirection = glm::vec3(0.0f);
    glm::vec3 lastPosition = glm::vec3(0.0f);
    

public:
    Enemy(const glm::vec3& position, const glm::vec3& rotation = {0.0f, 0.0f, 0.0f})
        : CharacterEntity("enemy", "gbuffer", position, rotation) {
        // Arrow shaft (long thin rectangle pointing forward in -Z)
        Entity* arrowShaft = new Entity(
            "enemy_arrow_shaft",
            "gbuffer",
            {0.0f, 2.5f, 0.0f},     // Raised up to be visible
            {0.0f, 0.0f, 0.0f},
            {0.2f, 0.2f, 3.0f},      // Very thin (0.2x0.2), long (3.0) in Z
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        arrowShaft->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(arrowShaft);

        // Arrow head (triangle-ish, pointing in -Z direction = forward)
        Entity* arrowHead = new Entity(
            "enemy_arrow_head",
            "gbuffer",
            {0.0f, 2.5f, -2.0f},     // At front of shaft
            {0.0f, 0.0f, 0.0f},
            {1.0f, 1.0f, 0.5f},      // Wide (1.0), short depth (0.5)
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        arrowHead->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(arrowHead);

        // Arrow tail fins (to make direction even more obvious)
        Entity* tailLeft = new Entity(
            "enemy_arrow_tail_left",
            "gbuffer",
            {-0.5f, 2.5f, 1.5f},     // Left side at back
            {0.0f, 0.0f, 0.0f},
            {0.8f, 0.2f, 0.3f},      // Horizontal fin
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        tailLeft->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(tailLeft);

        Entity* tailRight = new Entity(
            "enemy_arrow_tail_right",
            "gbuffer",
            {0.5f, 2.5f, 1.5f},      // Right side at back
            {0.0f, 0.0f, 0.0f},
            {0.8f, 0.2f, 0.3f},      // Horizontal fin
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        tailRight->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(tailRight);

        OBBCollider* obbBox = new OBBCollider(
            {0.0f, 0.6f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            this->getName(),
            {0.5f, 1.8f, 0.5f}
        );
        this->addChild(obbBox);

        detectorAABB = new DetectorAABB(
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            this->getName(),
            {10.0f, 10.0f, 10.0f}
        );
        this->addChild(detectorAABB);
        lastPosition = getPosition();
    }

private:
    enum class State {
        IDLE,
        COMBAT
    };
    State currentState = State::IDLE;

    // Combat parameters
    const float combatRadius = 5.0f;
    const float combatRadiusTolerance = 0.5f;

    // Strafe behavior
    float strafeTime = 1.5f;
    float strafeTimer = 0.0f;
    bool strafeLeft = true;

    // Combat timers
    float shootCooldown = 0.5f;
    float shootTimer = 0.0f;

    // Jumping
    float jumpCooldown = 1.0f;  // Jump at most once per second
    float jumpTimer = 0.0f;

    float rotationSpeed = 180.0f;

    void facePlayer(const glm::vec3& toPlayer, float deltaTime) {
        // Calculate target yaw from vector to player
        float targetYaw = glm::degrees(std::atan2(toPlayer.x, -toPlayer.z));
        float currentYaw = getRotation().y;
        float deltaYaw = targetYaw - currentYaw;

        // Normalize angle to [-180, 180]
        while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
        while (deltaYaw < -180.0f) deltaYaw += 360.0f;

        // Debug logging for rotation
        static int rotLogCount = 0;
        if (++rotLogCount % 60 == 0) {
            std::cout << "[Enemy Rotation Debug]\n"
                      << "  Direction to Player: X=" << toPlayer.x << " Z=" << toPlayer.z << "\n"
                      << "  Target Yaw: " << targetYaw << "°\n"
                      << "  Current Yaw: " << currentYaw << "°\n"
                      << "  Delta Yaw: " << deltaYaw << "°"
                      << std::endl;
        }

        // Clamp to max rotation speed
        deltaYaw = std::clamp(deltaYaw, -rotationSpeed * deltaTime, rotationSpeed * deltaTime);
        rotate({0.0f, deltaYaw, 0.0f});
    }

    void updateIdle(Entity* playerEntity, float deltaTime, const glm::vec3& currentMoveDir) {
        // Clear any movement from previous frame (IDLE state should never move)
        if (glm::length(currentMoveDir) > 0.001f) {
            stopMove(currentMoveDir);
            lastMoveDirection = glm::vec3(0.0f);
        }

        // Check if player entered detection box
        if (detectorAABB->isEntityInside(playerEntity)) {
            std::cout << "[Enemy] IDLE -> COMBAT: Player detected!" << std::endl;
            currentState = State::COMBAT;
            strafeTimer = 0.0f;
            strafeTime = 1.0f + (rand() % 1000) / 1000.0f;
            strafeLeft = (rand() % 2) == 0;
            shootTimer = 0.0f;
            jumpTimer = 0.0f;
        }
    }


    void updateCombat(Entity* playerEntity, const glm::vec3& toPlayer, float distanceXZ, float deltaTime, const glm::vec3& currentMoveDir) {
        // Clear previous frame's movement (except during jumps to maintain momentum)
        if (glm::length(currentMoveDir) > 0.001f && jumpTimer > 0.5f) {
            stopMove(currentMoveDir);
        }

        // Check if player left detection box
        if (!detectorAABB->isEntityInside(playerEntity)) {
            std::cout << "[Enemy] COMBAT -> IDLE: Player left detection box" << std::endl;

            lastMoveDirection = glm::vec3(0.0f);
            stopMove(currentMoveDir);
            currentState = State::IDLE;
            return;
        }
        strafeTimer += deltaTime;
        jumpTimer += deltaTime;
        shootTimer += deltaTime;

        // Movement logic: approach, retreat, or strafe based on distance
        glm::vec3 toPlayerNorm = glm::normalize(toPlayer);
        glm::vec3 worldDir;

        if (distanceXZ > combatRadius + combatRadiusTolerance) {
            // Too far - move toward player
            worldDir = toPlayerNorm;
        } else if (distanceXZ < combatRadius - combatRadiusTolerance) {
            // Too close - move away from player
            worldDir = -toPlayerNorm;
        } else {
            // At combat radius - strafe perpendicular to player
            if (strafeLeft) {
                worldDir = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), toPlayerNorm);
            } else {
                worldDir = glm::cross(toPlayerNorm, glm::vec3(0.0f, 1.0f, 0.0f));
            }
        }

        // Convert world direction to local space using inverse transform
        float yawRad = glm::radians(getRotation().y);
        glm::mat4 invYawRotation = glm::rotate(glm::mat4(1.0f), -yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 localDir = glm::vec3(invYawRotation * glm::vec4(worldDir, 0.0f));
        localDir.y = 0.0f;

        // Debug logging for movement direction
        static int movLogCount = 0;
        if (++movLogCount % 60 == 0) {
            const char* moveType = (distanceXZ > combatRadius + combatRadiusTolerance) ? "APPROACH" :
                                   (distanceXZ < combatRadius - combatRadiusTolerance) ? "RETREAT" : "STRAFE";
            std::cout << "[Enemy Movement Debug]\n"
                      << "  Movement Type: " << moveType << "\n"
                      << "  World Direction: X=" << worldDir.x << " Z=" << worldDir.z << "\n"
                      << "  Current Yaw: " << getRotation().y << "°\n"
                      << "  Local Direction: X=" << localDir.x << " Z=" << localDir.z
                      << std::endl;
        }

        lastMoveDirection = localDir;
        if (glm::length(lastMoveDirection) > 0.001f) {
            lastMoveDirection = glm::normalize(lastMoveDirection);
            move(lastMoveDirection);
        }

        // Detect if stuck (trying to move but not going anywhere) and jump over obstacle
        glm::vec3 currentPos = getPosition();
        glm::vec3 positionDelta = currentPos - lastPosition;
        positionDelta.y = 0.0f;  // Only care about XZ movement
        float movementThisFrame = glm::length(positionDelta);

        // If we're trying to move but barely moving (stuck on obstacle)
        if (glm::length(lastMoveDirection) > 0.001f && movementThisFrame < 0.08f) {
            // We're stuck - try to jump
            if (jumpTimer >= jumpCooldown) {
                std::cout << "[Enemy] COMBAT: Jumping over obstacle (moved " << movementThisFrame << " units)" << std::endl;
                jump();
                jumpTimer = 0.0f;
            }
        }

        lastPosition = currentPos;

        // Shoot at player
        if (shootTimer >= shootCooldown) {
            std::cout << "[Enemy] COMBAT: SHOOT! (Distance: " << distanceXZ << ")" << std::endl;
            shoot();
            shootTimer = 0.0f;
        }

        // Change strafe direction periodically
        if (strafeTimer >= strafeTime) {
            strafeLeft = !strafeLeft;
            strafeTimer = 0.0f;
            strafeTime = 1.0f + (rand() % 1000) / 1000.0f;
            std::cout << "[Enemy] COMBAT: Switching strafe direction" << std::endl;
        }
    }

    void shoot() {
        Entity* playerEntity = EntityManager::getInstance()->getEntity("player");
    }

public:
    void update(float deltaTime) override {
        static int frameCount = 0;
        frameCount++;

        Entity* playerEntity = EntityManager::getInstance()->getEntity("player");
        if (!playerEntity) {
            CharacterEntity::update(deltaTime);
            return;
        }

        // Calculate distances
        glm::vec3 playerPos = playerEntity->getPosition();
        glm::vec3 myPos = getPosition();
        glm::vec3 toPlayer = playerPos - myPos;
        glm::vec3 toPlayerXZ = toPlayer;
        toPlayerXZ.y = 0.0f;  // Ignore vertical distance for XZ calculations
        float distanceXZ = glm::length(toPlayerXZ);

        // Log every 60 frames (roughly once per second at 60fps)
        if (frameCount % 60 == 0) {
            const char* stateStr = (currentState == State::IDLE) ? "IDLE" : "COMBAT";
            std::cout << "[Enemy] Frame: " << frameCount << " | State: " << stateStr << "\n"
                      << "  Enemy Position: X=" << myPos.x << " Y=" << myPos.y << " Z=" << myPos.z << "\n"
                      << "  Player Position: X=" << playerPos.x << " Y=" << playerPos.y << " Z=" << playerPos.z << "\n"
                      << "  Delta to Player: X=" << toPlayer.x << " Y=" << toPlayer.y << " Z=" << toPlayer.z << "\n"
                      << "  Distance to Player (XZ): " << distanceXZ << " units"
                      << std::endl;
        }

        // Face the player FIRST (only in COMBAT state)
        // This ensures rotation is updated before movement calculation
        if (currentState == State::COMBAT) {
            facePlayer(toPlayerXZ, deltaTime);
        }

        // Run AI state machine (states handle their own movement clearing)
        switch (currentState) {
            case State::IDLE:
                updateIdle(playerEntity, deltaTime, lastMoveDirection);
                break;

            case State::COMBAT:
                updateCombat(playerEntity, toPlayerXZ, distanceXZ, deltaTime, lastMoveDirection);
                break;
        }

        // Call parent update to handle physics and movement
        CharacterEntity::update(deltaTime);
    }
};


