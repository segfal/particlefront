#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <variant>
#include "Image.h"

class Renderer;
class Shader;
class TextureManager;
class TextObject;

class UIObject {
private:
    std::string name;
    glm::vec2 position;
    glm::vec2 size;
    glm::ivec2 corner;
    std::string texture;
    std::vector<VkDescriptorSet> descriptorSets;
    bool enabled = true;
    
public:
    std::map<std::string, std::variant<UIObject*, TextObject*>> children;

    UIObject(glm::vec2 position,
             glm::vec2 size,
             glm::ivec2 corner,
             std::string name,
             std::string texture)
        : name(std::move(name)),
          position(position),
          size(size),
          corner(corner),
          texture(std::move(texture)) {}

    virtual ~UIObject() {
        for (auto &entry : children) {
            std::visit([](auto* child) { delete child; }, entry.second);
        }
    }

    const std::string &getName() const { return name; }
    const glm::vec2 &getPosition() const { return position; }
    const glm::vec2 &getSize() const { return size; }
    const glm::ivec2 &getCorner() const { return corner; }
    bool isEnabled() const { return enabled; }
    const std::string &getTexture() const { return texture; }

    void loadTexture();

    const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }

    void addChild(UIObject *child) { children[child->getName()] = child; }
    void addChild(TextObject *child) { children[child->getName()] = child; }
    void removeChild(UIObject *child) {
        auto it = children.find(child->getName());
        if (it != children.end()) {
            std::visit([](auto* child) { delete child; }, it->second);
            children.erase(it);
        }
    }
};
