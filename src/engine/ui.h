#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <map>

class UIObject {
private:
    std::string name;
    glm::vec2 position;
    glm::vec2 size;
    glm::ivec2 corner;
    VkImage texture;
    VkImageView textureView;
    VkDeviceMemory textureMemory;
    VkSampler textureSampler;
    VkFormat format;
    std::map<std::string, UIObject*> children;
public:
    UIObject(glm::vec2 position,
             glm::vec2 size,
             glm::ivec2 corner,
             std::string name,
             VkImage texture,
             VkImageView textureView,
             VkDeviceMemory textureMemory,
             VkSampler textureSampler,
             VkFormat format)
        : name(std::move(name)),
          position(position),
          size(size),
          corner(corner),
          texture(texture),
          textureView(textureView),
          textureMemory(textureMemory),
          textureSampler(textureSampler),
          format(format) {}

    ~UIObject() {
        for (auto &entry : children) {
            delete entry.second;
        }
    }

    const std::string &getName() const { return name; }
    const glm::vec2 &getPosition() const { return position; }
    const glm::vec2 &getSize() const { return size; }
    const glm::ivec2 &getCorner() const { return corner; }

    void addChild(UIObject *child) {
        children[child->getName()] = child;
    }
    void removeChild(UIObject *child) {
        auto it = children.find(child->getName());
        if (it != children.end()) {
            delete it->second;
            children.erase(it);
        }
    }
};
