#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <vector>
#include "Image.h"

class Renderer;
struct Shader;
class TextureManager;

class UIObject {
private:
    std::string name;
    glm::vec2 position;
    glm::vec2 size;
    glm::ivec2 corner;
    std::string texture;
    std::vector<VkDescriptorSet> descriptorSets;
    UIObject* parent = nullptr;
    bool enabled = true;
    
public:
    std::map<std::string, UIObject*> children;

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

    virtual ~UIObject();

    const std::string &getName() const { return name; }
    const glm::vec2 &getPosition() const { return position; }
    const glm::vec2 &getSize() const { return size; }
    const glm::ivec2 &getCorner() const { return corner; }
    bool isEnabled() const { return enabled; }
    const std::string &getTexture() const { return texture; }

    void loadTexture();

    const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }

    void addChild(UIObject *child);
    void removeChild(UIObject *child);
    UIObject* getParent() const { return parent; }
};

class TextObject : public UIObject {
public:
    TextObject(
        std::string text,
        std::string font,
        glm::vec2 position,
        glm::vec2 size,
        glm::ivec2 corner,
        std::string name,
        glm::vec3 color
    );
    std::string text;
    std::string font;
    glm::vec3 color;
};
