#pragma once
#include "Entity.h"

class Camera : public Entity {
public:
    Camera(glm::vec3 position, glm::vec3 rotation, float fov) : Entity("camera", "", position, rotation), fov(fov) {}
    void update(float deltaTime) override {}
    float getFOV() const { return fov; }
private:
    float fov;
};