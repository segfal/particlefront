#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <map>
#include "Image.h"
#include "UIObject.cpp"

class UIManager {
private:
    std::map<std::string, UIObject*> uiObjects;
    std::map<std::string, Image> textures;
public:
    UIManager() = default;
    ~UIManager() {
        for(auto &entry : uiObjects) {
            delete entry.second;
        }
    }
    void addUIObject(UIObject* obj) {
        uiObjects[obj->getName()] = obj;
    }
    void removeUIObject(UIObject* obj) {
        auto it = uiObjects.find(obj->getName());
        if(it != uiObjects.end()) {
            delete it->second;
            uiObjects.erase(it);
        }
    }
    UIObject* getUIObject(const std::string& name) {
        auto it = uiObjects.find(name);
        if(it != uiObjects.end()) {
            return it->second;
        }
        return nullptr;
    }
    void addTexture(const std::string& name, const Image& image) {
        textures[name] = image;
    }
    void removeTexture(const std::string& name) {
        auto it = textures.find(name);
        if(it != textures.end()) {
            textures.erase(it);
        }
    }
    Image* getTexture(const std::string& name) {
        auto it = textures.find(name);
        if(it != textures.end()) {
            return &it->second;
        }
        return nullptr;
    }
    static UIManager* getInstance() {
        static UIManager instance;
        return &instance;
    }
    static uint32_t getUIObjectCount() {
        return static_cast<uint32_t>(getInstance()->uiObjects.size());
    }
};