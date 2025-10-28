#include <Frustrum.h>

void Frustum::extractFromMatrix(const glm::mat4& viewProjectionMatrix) {
    planes[LEFT].normal = glm::vec3(
        viewProjectionMatrix[0][3] + viewProjectionMatrix[0][0],
        viewProjectionMatrix[1][3] + viewProjectionMatrix[1][0],
        viewProjectionMatrix[2][3] + viewProjectionMatrix[2][0]
    );
    planes[LEFT].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][0];

    planes[RIGHT].normal = glm::vec3(
        viewProjectionMatrix[0][3] - viewProjectionMatrix[0][0],
        viewProjectionMatrix[1][3] - viewProjectionMatrix[1][0],
        viewProjectionMatrix[2][3] - viewProjectionMatrix[2][0]
    );
    planes[RIGHT].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][0];

    planes[TOP].normal = glm::vec3(
        viewProjectionMatrix[0][3] - viewProjectionMatrix[0][1],
        viewProjectionMatrix[1][3] - viewProjectionMatrix[1][1],
        viewProjectionMatrix[2][3] - viewProjectionMatrix[2][1]
    );

    planes[TOP].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][1];

    planes[BOTTOM].normal = glm::vec3(
        viewProjectionMatrix[0][3] + viewProjectionMatrix[0][1],
        viewProjectionMatrix[1][3] + viewProjectionMatrix[1][1],
        viewProjectionMatrix[2][3] + viewProjectionMatrix[2][1]
    );

    planes[BOTTOM].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][1];

    planes[NEAR].normal = glm::vec3(
        viewProjectionMatrix[0][3] + viewProjectionMatrix[0][2],
        viewProjectionMatrix[1][3] + viewProjectionMatrix[1][2],
        viewProjectionMatrix[2][3] + viewProjectionMatrix[2][2]
    );

    planes[NEAR].distance = viewProjectionMatrix[3][3] + viewProjectionMatrix[3][2];

    planes[FAR].normal = glm::vec3(
        viewProjectionMatrix[0][3] - viewProjectionMatrix[0][2],
        viewProjectionMatrix[1][3] - viewProjectionMatrix[1][2],
        viewProjectionMatrix[2][3] - viewProjectionMatrix[2][2]
    );

    planes[FAR].distance = viewProjectionMatrix[3][3] - viewProjectionMatrix[3][2];
    
    for(int i=0;i<6;i++) {
        float length = glm::length(planes[i].normal);
        if(length > 0.0f){
            planes[i].normal /= length;
            planes[i].distance /= length;
        
        }
    }

}

bool Frustum::intersectsAABB(const glm::vec3& min,const glm::vec3& max) const {
    for(int i=0; i<6; i++) {
        glm::vec3 pVertex(
            planes[i].normal.x > 0.0f ? max.x : min.x,
            planes[i].normal.y > 0.0f ? max.y : min.y,
            planes[i].normal.z > 0.0f ? max.z : min.z
        );
        if(planes[i].distanceToPoint(pVertex) < 0.0f) {
            return false;
        }
    }

    return true;
}