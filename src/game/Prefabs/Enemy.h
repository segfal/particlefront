#pragma once
#include <CharacterEntity.h>
#include <EntityManager.h>
#include <Collider.h>

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
};

class Enemy : public CharacterEntity {
private:
    DetectorAABB* detectorAABB = nullptr;
    glm::vec3 lastMoveDirection = glm::vec3(0.0f);
    glm::vec3 lastPosition = glm::vec3(0.0f);
    glm::vec3 computedWorldDirection = glm::vec3(0.0f);
    glm::vec3 lastPlayerPosition = glm::vec3(0.0f);
    bool hasLastPlayerPosition = false;
    const float playerMoveThreshold = 0.02f;

public:
    Enemy(const glm::vec3& position, const glm::vec3& rotation = {0.0f, 0.0f, 0.0f}, float customSpeed = 6.0f)
        : CharacterEntity("enemy", "gbuffer", position, rotation), chaseSpeed(customSpeed) {
      
        Entity* arrowShaft = new Entity(
            "enemy_arrow_shaft",
            "gbuffer",
            {0.0f, 2.5f, 0.0f},   
            {0.0f, 0.0f, 0.0f},
            {0.2f, 0.2f, 3.0f},    
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        arrowShaft->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(arrowShaft);

      
        Entity* arrowHead = new Entity(
            "enemy_arrow_head",
            "gbuffer",
            {0.0f, 2.5f, -2.0f},   
            {0.0f, 0.0f, 0.0f},
            {1.0f, 1.0f, 0.5f},    
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        arrowHead->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(arrowHead);

      
        Entity* tailLeft = new Entity(
            "enemy_arrow_tail_left",
            "gbuffer",
            {-0.5f, 2.5f, 1.5f},   
            {0.0f, 0.0f, 0.0f},
            {0.8f, 0.2f, 0.3f},    
            {"materials_default_albedo", "materials_default_metallic", "materials_default_roughness", "materials_default_normal"}
        );
        tailLeft->setModel(ModelManager::getInstance()->getModel("cube"));
        this->addChild(tailLeft);

        Entity* tailRight = new Entity(
            "enemy_arrow_tail_right",
            "gbuffer",
            {0.5f, 2.5f, 1.5f},    
            {0.0f, 0.0f, 0.0f},
            {0.8f, 0.2f, 0.3f},    
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
        setMoveSpeed(chaseSpeed);
    }

private:
    enum class State {
        IDLE,
        COMBAT
    }; 
    State currentState = State::COMBAT; 


    const float combatRadius = 5.0f;
    const float combatRadiusTolerance = 1.0f;


    
    float strafeAngle = 0.0f; 
    float strafeAngleSpeed = 30.0f; 
    bool strafeLeft = true;
    const float maxStrafeAngle = 85.0f; 
    const float minStrafeAngle = -85.0f;


    float shootCooldown = 0.5f;
    float shootTimer = 0.0f;


    float jumpCooldown = 0.5f;
    float jumpTimer = 0.0f;

    float rotationSpeed = 90.0f;
    const float stuckMovementThreshold = 0.05f;
    const int maxStuckFrames = 10;
    int stuckFrameCount = 0;
    float chaseSpeed = 6.0f;
    void haltMovement() {
        if (glm::length(lastMoveDirection) > 0.001f) {
            stopMove(lastMoveDirection);
        }
        lastMoveDirection = glm::vec3(0.0f);
        computedWorldDirection = glm::vec3(0.0f);
        stuckFrameCount = 0;
        resetVelocity();
    }

    void facePlayer(const glm::vec3& toPlayer, float deltaTime) {
      
        float targetYaw = glm::degrees(std::atan2(toPlayer.x, -toPlayer.z));
        float currentYaw = getRotation().y;
        float deltaYaw = targetYaw - currentYaw;

      
        while (deltaYaw > 180.0f) deltaYaw -= 360.0f;
        while (deltaYaw < -180.0f) deltaYaw += 360.0f;

      
      
        deltaYaw = std::clamp(deltaYaw, -rotationSpeed * deltaTime, rotationSpeed * deltaTime);
        rotate({0.0f, deltaYaw, 0.0f});
    }

    void updateIdle(Entity* playerEntity, float deltaTime) {
        computedWorldDirection = glm::vec3(0.0f);

        if (detectorAABB->isEntityInside(playerEntity)) {
            currentState = State::COMBAT;
            strafeAngle = 0.0f; 
            strafeLeft = (rand() % 2) == 0;
            shootTimer = 0.0f;
            jumpTimer = 0.0f;
        }
    }


    void updateCombatLogic(Entity* playerEntity, const glm::vec3& toPlayer, float distanceXZ, float deltaTime, bool isPlayerMoving) {
        jumpTimer += deltaTime;
        shootTimer += deltaTime;

        
        glm::vec3 playerPos = playerEntity->getPosition();
        float playerYaw = playerEntity->getRotation().y;

        
        if (strafeLeft) {
            strafeAngle -= strafeAngleSpeed * deltaTime;
            if (strafeAngle <= minStrafeAngle) {
                strafeAngle = minStrafeAngle;
                strafeLeft = false; 
            }
        } else {
            strafeAngle += strafeAngleSpeed * deltaTime;
            if (strafeAngle >= maxStrafeAngle) {
                strafeAngle = maxStrafeAngle;
                strafeLeft = true; 
            }
        }

        
        
        float targetAngleWorld = playerYaw + strafeAngle;
        float targetAngleRad = glm::radians(targetAngleWorld);

        
        
        glm::vec3 offsetDirection = glm::vec3(
            sin(targetAngleRad),
            0.0f,
            -cos(targetAngleRad)
        );

        glm::vec3 targetPosition = playerPos + offsetDirection * combatRadius;
        targetPosition.y = playerPos.y; 

        
        glm::vec3 myPos = getPosition();
        glm::vec3 toTarget = targetPosition - myPos;

        
        float yDifference = toTarget.y;
        toTarget.y = 0.0f; 

        float distanceToTarget = glm::length(toTarget);

        
        if (distanceToTarget > 0.1f) {
            computedWorldDirection = glm::normalize(toTarget);
        } else {
            computedWorldDirection = glm::vec3(0.0f);
        }

        
        
        if (std::abs(yDifference) > 0.5f) {
            glm::vec3 correctedPos = myPos;
            correctedPos.y = playerPos.y;
            setPosition(correctedPos);
        }

        
        if (shootTimer >= shootCooldown) {
            shoot();
            shootTimer = 0.0f;
        }
    }

    void applyComputedMovement(float deltaTime) {
        
        if (glm::length(lastMoveDirection) > 0.001f) {
            stopMove(lastMoveDirection);
            lastMoveDirection = glm::vec3(0.0f);
        }

        
        float yawRad = glm::radians(getRotation().y);
        glm::mat4 invYawRotation = glm::rotate(glm::mat4(1.0f), -yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 localDir = glm::vec3(invYawRotation * glm::vec4(computedWorldDirection, 0.0f));
        localDir.y = 0.0f;

        
        if (glm::length(computedWorldDirection) > 0.001f && jumpTimer >= jumpCooldown) {
            
            float lookAheadDist = 1.5f; 
            glm::vec3 lookAheadPos = computedWorldDirection * lookAheadDist;
            lookAheadPos.y = 0.0f; 

            collision futureCollision = willCollide(lookAheadPos);

            if (futureCollision.other) {
                
                float mtvLen = glm::length(futureCollision.mtv);
                if (mtvLen > 0.001f) {
                    glm::vec3 collisionNormal = futureCollision.mtv / mtvLen;

                    
                    
                    if (std::abs(collisionNormal.y) < 0.7f) {
                        jump();
                        jumpTimer = 0.0f;
                        stuckFrameCount = 0;
                    }
                }
            }
        }

        
        if (glm::length(localDir) > 0.001f) {
            lastMoveDirection = glm::normalize(localDir);
            move(lastMoveDirection);
        }

       
        glm::vec3 currentPos = getPosition();
        glm::vec3 positionDelta = currentPos - lastPosition;
        positionDelta.y = 0.0f;
        float movementThisFrame = glm::length(positionDelta);

        bool tryingToMove = glm::length(lastMoveDirection) > 0.001f;
        if (tryingToMove && movementThisFrame < stuckMovementThreshold) {
            stuckFrameCount++;
        } else {
            stuckFrameCount = 0;
        }

       
        if (tryingToMove && stuckFrameCount >= maxStuckFrames) {
            if (jumpTimer >= jumpCooldown) {
                jump();
                jumpTimer = 0.0f;
                stuckFrameCount = 0;
            }
        }

        lastPosition = currentPos;
    }

    void shoot() {
        std::cout << "[Enemy] Shooting at player!" << std::endl;
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

      
        glm::vec3 playerPos = playerEntity->getPosition();
        glm::vec3 myPos = getPosition();

        bool isPlayerMoving = false;
        if (hasLastPlayerPosition) {
            glm::vec3 playerDelta = playerPos - lastPlayerPosition;
            playerDelta.y = 0.0f;
            isPlayerMoving = glm::length(playerDelta) > playerMoveThreshold;
        } else {
            hasLastPlayerPosition = true;
        }
        lastPlayerPosition = playerPos;

        glm::vec3 toPlayer = playerPos - myPos;
        glm::vec3 toPlayerXZ = toPlayer;
        toPlayerXZ.y = 0.0f;
        float distanceXZ = glm::length(toPlayerXZ);

      
      
        if (currentState == State::COMBAT) {
            facePlayer(toPlayerXZ, deltaTime);
        }

        switch (currentState) {
            case State::IDLE:
                updateIdle(playerEntity, deltaTime);
                break;

            case State::COMBAT:
                updateCombatLogic(playerEntity, toPlayerXZ, distanceXZ, deltaTime, isPlayerMoving);
                break;
        }

        applyComputedMovement(deltaTime);
        CharacterEntity::update(deltaTime);
    }
};
