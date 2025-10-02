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
public:
    UIManager() = default;
    ~UIManager() {
        for(auto &entry : uiObjects) {
            delete entry.second;
        }
    }
    void loadTextures() {
        for(auto &entry : uiObjects) {
            entry.second->loadTexture();
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
    static UIManager* getInstance() {
        static UIManager instance;
        return &instance;
    }
    static uint32_t getUIObjectCount() {
        return static_cast<uint32_t>(getInstance()->uiObjects.size());
    }
};