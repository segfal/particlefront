#pragma once
#include "Entity.h"

class Camera : public Entity {
public:
    Camera(glm::vec3 position, glm::vec3 rotation) : Entity("camera", "", position, rotation) {}
    void update(float deltaTime) override {}
};