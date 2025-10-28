#include <CharacterEntity.h>
#include <algorithm>
#include <cmath>
#include <Entity.h>
#include <Collider.h>
#include <glm/gtc/matrix_transform.hpp>

void CharacterEntity::update(float deltaTime) {
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

    bool touchedGroundThisFrame = false;
    glm::vec3 groundNormalAccum(0.0f);
    constexpr float kSkin = 0.002f; // small separation to avoid re-colliding due to numerical error
    for (int i = 0; i < steps; ++i) {
        glm::vec3 vStep(0.0f, velocity.y * subDt, 0.0f);
        if (vStep.y != 0.0f) {
            collision vcol = willCollide(vStep);
            if (!vcol.other) {
                setPosition(getPosition() + vStep);
            } else {
                setPosition(getPosition() + vStep + vcol.mtv);
                float len = glm::length(vcol.mtv);
                if (len > 1e-5f) {
                    glm::vec3 n = vcol.mtv / len;
                    float vn = glm::dot(velocity, n);
                    if (vn < 0.0f) velocity -= vn * n;
                    setPosition(getPosition() + n * kSkin);
                    if (n.y > groundedNormalThreshold && velocity.y <= 0.0f) {
                        velocity.y = 0.0f;
                        touchedGroundThisFrame = true;
                        groundNormalAccum += n;
                    }
                }
            }
        }
        if (vStep.y == 0.0f) {
            const float kGroundProbe = 0.02f;
            collision gcol = willCollide(glm::vec3(0.0f, -kGroundProbe, 0.0f));
            if (gcol.other) {
                float glen = glm::length(gcol.mtv);
                if (glen > 1e-5f) {
                    glm::vec3 gn = gcol.mtv / glen;
                    if (gn.y > groundedNormalThreshold) {
                        touchedGroundThisFrame = true;
                        groundNormalAccum += gn;
                        if (velocity.y < 0.0f) velocity.y = 0.0f;
                    }
                }
            }
        }
        glm::vec3 hStep(velocity.x * subDt, 0.0f, velocity.z * subDt);
        if (hStep.x != 0.0f || hStep.z != 0.0f) {
            if (touchedGroundThisFrame) {
                glm::vec3 gN = glm::length(groundNormalAccum) > 0.0f ? glm::normalize(groundNormalAccum) : glm::vec3(0.0f, 1.0f, 0.0f);
                hStep = hStep - glm::dot(hStep, gN) * gN;
            }
            collision hcol = willCollide(hStep);
            if (!hcol.other) {
                setPosition(getPosition() + hStep);
            } else {
                glm::vec3 mtv = hcol.mtv;
                if (!touchedGroundThisFrame && mtv.y > 0.0f) {
                    mtv.y = 0.0f;
                }
                setPosition(getPosition() + hStep + mtv);
                float len = glm::length(hcol.mtv);
                if (len > 1e-5f) {
                    glm::vec3 n = hcol.mtv / len;
                    float vn = glm::dot(velocity, n);
                    if (vn < 0.0f) velocity -= vn * n;
                    setPosition(getPosition() + n * kSkin);
                }
            }
        }
        collision post = willCollide(glm::vec3(0.0f));
        if (post.other) {
            setPosition(getPosition() + post.mtv);
            float len = glm::length(post.mtv);
            if (len > 1e-5f) {
                glm::vec3 fix = post.mtv;
                if (!touchedGroundThisFrame && fix.y > 0.0f) {
                    fix.y = 0.0f;
                    setPosition(getPosition() - post.mtv + fix);
                }
                glm::vec3 n = (len > 0.0f) ? (post.mtv / len) : glm::vec3(0.0f,1.0f,0.0f);
                float vn = glm::dot(velocity, n);
                if (vn < 0.0f) velocity -= vn * n;
                setPosition(getPosition() + n * kSkin);
            }
        }
    }
    if (touchedGroundThisFrame) {
        grounded = true;
        groundedTimer = 0.0f;
    } else {
        grounded = groundedTimer <= coyoteTime;
        groundedTimer += deltaTime;
    }
    if (glm::length(velocity) < 1e-4f) {
        resetVelocity();
    }
}
void CharacterEntity::move(const glm::vec3& delta) {
    pressed += delta;
}
void CharacterEntity::stopMove(const glm::vec3& delta) {
    pressed -= delta;
    if (glm::length(pressed) < 0.001f) {
        resetVelocity();
    }
}
void CharacterEntity::jump() {
    if (grounded || groundedTimer <= coyoteTime) {
        velocity.y = jumpSpeed;
        grounded = false;
    }
}
void CharacterEntity::resetVelocity() {
    velocity = glm::vec3(0.0f);
}
void CharacterEntity::rotate(const glm::vec3& delta) {
    if (delta.y != 0.0f) {
        glm::vec3 bodyRot = getRotation();
        bodyRot.y = std::fmod(bodyRot.y + delta.y, 360.0f);
        if (bodyRot.y < 0.0f) bodyRot.y += 360.0f;
        collision rotCol = willCollide(glm::vec3(0.0f), bodyRot - getRotation());
        const float kYawPenetrationEps = 2.0e-2f;
        if (!rotCol.other) {
            setRotation(bodyRot);
        } else {
            float pen = glm::length(rotCol.mtv);
            if (pen < kYawPenetrationEps) {
                setRotation(bodyRot);
            } else {
                glm::vec3 n = rotCol.mtv / pen;
                if (std::abs(n.y) > 0.6f) {
                    setRotation(bodyRot);
                }
            }
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

collision CharacterEntity::willCollide(const glm::vec3& deltaPos, const glm::vec3& deltaRot) {
    OBBCollider* myBox = nullptr;
    for (auto& child : this->getChildren()) {
        myBox = dynamic_cast<OBBCollider*>(child);
        if (myBox) break;
    }
    if (!myBox) {
        return {nullptr, glm::vec3(0.0f)};
    }
    ColliderAABB myAABB = myBox->getWorldAABB();
    if (deltaPos.x != 0.0f || deltaPos.y != 0.0f || deltaPos.z != 0.0f) {
        myAABB.min += deltaPos;
        myAABB.max += deltaPos;
    }

    CollisionMTV mtv{};
    constexpr float kMTV_MIN_LEN = 1e-3f;
    constexpr float kPENETRATION_MIN = 1e-4f;
    EntityManager* entityMgr = EntityManager::getInstance();
    for (const auto& [otherName, otherEntity] : entityMgr->getAllEntities()) {
        if (otherEntity == this) continue;
        Collider* otherCollider = nullptr;
        for (auto& child : otherEntity->getChildren()) {
            otherCollider = dynamic_cast<Collider*>(child);
            if (otherCollider) break;
        }
        if (!otherCollider) continue;
        ColliderAABB otherAABB = otherCollider->getWorldAABB();
        float broadphaseMargin = (glm::length(deltaRot) > 0.0f) ? 0.0f : 0.005f;
        bool aabbOverlaps = aabbIntersects(myAABB, otherAABB, broadphaseMargin);
        if (!aabbOverlaps) continue;
        
        mtv = CollisionMTV{};
        if (myBox->intersectsMTV(*otherCollider, mtv, deltaPos, deltaRot)) {
            if (mtv.penetration > kPENETRATION_MIN && glm::length(mtv.mtv) > kMTV_MIN_LEN) {
                return {otherCollider, mtv.mtv};
            }
        }
    }
    return {nullptr, glm::vec3(0.0f)};
}