#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "TextureManager.h"

class Model;

class Entity {
public:
    Entity(std::string name, std::string shader, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale = glm::vec3(1.0f), std::vector<std::string> textures = {}) : name(std::move(name)), shader(std::move(shader)), position(position), rotation(rotation), scale(scale), textures(std::move(textures)) {
        loadTextures();
    }
    ~Entity() {
        for (auto& child : children) {
            delete child;
        }
        children.clear();
    }
    virtual void update(float deltaTime) {}

    void addChild(Entity* child) {
        if (child->parent) {
            child->parent->removeChild(child);
        }
        children.push_back(child);
        child->parent = this;
    }
    void removeChild(Entity* child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
        child->parent = nullptr;
    }
    void moveToParent(Entity* newParent) {
        if (parent) {
            parent->removeChild(this);
        }
        if (newParent) {
            newParent->addChild(this);
        }
    }

    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { position = pos; }
    glm::vec3 getRotation() const { return rotation; }
    void setRotation(const glm::vec3& rot) { rotation = rot; }
    glm::vec3 getScale() const { return scale; }
    void setScale(const glm::vec3& s) { scale = s; }
    std::string getName() const { return name; }
    std::string getShader() const { return shader; }
    bool isActive() const { return active; }
    void setActive(bool state) { active = state; }
    Model* getModel() const { return model; }
    void setModel(Model* m) { model = m; }
    std::vector<Entity*>& getChildren() { return children; }
    Entity* getParent() const { return parent; }

    void loadTextures() {
        if (textures.empty()) return;
        TextureManager* texMgr = TextureManager::getInstance();
        Renderer* renderer = Renderer::getInstance();
        Shader* shaderUsed = renderer->getShaderManager()->getShader(shader);
        if (!shaderUsed) {
            std::cerr << "Shader " << shader << " not found!" << std::endl;
            return;
        }
        std::vector<Image*> textureResources;
        if (shader == "pbr") {
            if (textures.size() > 4) {
                std::cerr << "PBR shader supports up to 4 textures (albedo, metallic, roughness, normal). Extra textures will be ignored." << std::endl;
            }
            int i = 0;
            std::vector<std::string> defaultTextures = {
                "default_albedo", "default_metallic", "default_roughness", "default_normal"
            };
            for (const auto& texture : textures) {
                Image* img = texMgr->getTexture(texture);
                if (!img) {
                    textureResources.push_back(texMgr->getTexture(defaultTextures[i]));
                } else {
                    textureResources.push_back(img);
                }
                i++;
            }
        } else for (const auto& texture : textures) {
            Image* img = texMgr->getTexture(texture);
            if (!img) {
                std::cerr << "Texture " << texture << " not found!" << std::endl;
            } else {
                textureResources.push_back(img);
            }
        }
        if (textureResources.empty()) {
            std::cerr << "No valid textures found!" << std::endl;
        } else {
            std::vector<Image*> textures = textureResources;
            std::vector<VkBuffer> buffers;
            descriptorSets = renderer->createDescriptorSets(
                shaderUsed->descriptorPool,
                shaderUsed->descriptorSetLayout,
                shaderUsed->vertexBitBindings,
                shaderUsed->fragmentBitBindings,
                textures,
                buffers
            );
        }
    }
    const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }
private:
    std::string name;
    std::string shader = "pbr";
    std::vector<std::string> textures;
    std::vector<VkDescriptorSet> descriptorSets;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    std::vector<Entity*> children;
    Entity* parent = nullptr;
    Model* model = nullptr;
    bool active = true;
};