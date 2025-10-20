#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <cstdint>

class Renderer;

class Model {
public:
    Model(std::string name)
        : name(std::move(name)) {};
    ~Model() = default;
    void loadFromFile(const std::string& path);
    const std::string& getName() const { return name; }
    VkBuffer getVertexBuffer() const { return vertexBuffer; }
    VkBuffer getIndexBuffer() const { return indexBuffer; }
    const uint32_t getIndexCount() const { return static_cast<uint32_t>(indices.size()); }
    const glm::vec3& getBoundsMin() const { return boundsMin; }
    const glm::vec3& getBoundsMax() const { return boundsMax; }
private:
    Renderer* renderer = nullptr;
    std::string name;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
};