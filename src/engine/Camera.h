#pragma once
#include "Entity.h"
#include "Frustrum.h"
class Camera : public Entity {
public:
    Camera(glm::vec3 position, glm::vec3 rotation, float fov) : Entity("camera", "", position, rotation), fov(fov) {}
    void update(float deltaTime) override {}
    float getFOV() const { return fov; }
    Frustum getFrustrum(float aspectRatio, float nearPlane, float farPlane, const glm::mat4& worldTransform) const; // todo task 5
private:
    float fov;
};

inline Frustum Camera::getFrustrum(
                            float aspectRatio,
                            float nearPlane = 0.1f,float farPlane = 100.0f,
                            const glm::mat4& worldTransform = glm::mat4(1.0f)) const {
    glm::mat4 view = glm::inverse(worldTransform);
    glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    projection[1][1] *= -1.0f;
    glm::mat4 viewProjection = projection * view;
    Frustum frustum;
    frustum.extractFromMatrix(viewProjection);
    return frustum;
}