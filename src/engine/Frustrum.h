#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



// Axis Aligned Bounding Box
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};




class Frustum {
    public:
        enum PlaneID {
            LEFT = 0,
            RIGHT,
            TOP,
            BOTTOM,
            NEAR,
            FAR
        
        };

        struct Plane {
            glm::vec3 normal;
            float distance;

            float distanceToPoint(const glm::vec3& point) const {
                return glm::dot(normal, point) + distance;
            }

        };

        Plane planes[6];

        void extractFromMatrix(const glm::mat4& viewProjectionMatrix);
        bool intersectsAABB(const glm::vec3& min, const glm::vec3& max) const;

};



