#include "Entity.h"
#include "Model.h"

AABB Entity::getWorldBounds(const glm::mat4& worldTransform) const {
    Model* model = getModel();
    if (!model) {
        glm::vec3 worldPos = glm::vec3(worldTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        return AABB{worldPos, worldPos};
    }

    glm::vec3 modelMax = model->getBoundsMax();
    glm::vec3 modelMin = model->getBoundsMin();

    glm::vec3 corners[8] = {
        glm::vec3(modelMin.x, modelMin.y, modelMin.z),
        glm::vec3(modelMax.x, modelMin.y, modelMin.z),
        glm::vec3(modelMin.x, modelMax.y, modelMin.z),
        glm::vec3(modelMax.x, modelMax.y, modelMin.z),
        glm::vec3(modelMin.x, modelMin.y, modelMax.z),
        glm::vec3(modelMax.x, modelMin.y, modelMax.z),
        glm::vec3(modelMin.x, modelMax.y, modelMax.z),
        glm::vec3(modelMax.x, modelMax.y, modelMax.z)
    };

    AABB worldAABB;
    worldAABB.min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    worldAABB.max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for(int i=0; i<8; i++) {
        glm::vec3 corner = glm::vec3(worldTransform * glm::vec4(corners[i], 1.0f));
        worldAABB.min = glm::min(worldAABB.min, corner);
        worldAABB.max = glm::max(worldAABB.max, corner);
    }
    return worldAABB;
}
