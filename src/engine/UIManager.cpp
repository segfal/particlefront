#include "UIManager.h"
#include "UIObject.h"

UIManager::~UIManager() {
    for(auto &entry : uiObjects) {
        delete entry.second;
    }
}

void UIManager::loadTextures() {
    for(auto &entry : uiObjects) {
        entry.second->loadTexture();
    }
}

void UIManager::addUIObject(UIObject* obj) { uiObjects[obj->getName()] = obj; }

void UIManager::removeUIObject(UIObject* obj) {
    auto it = uiObjects.find(obj->getName());
    if(it != uiObjects.end()) {
        delete it->second;
        uiObjects.erase(it);
    }
}

UIObject* UIManager::getUIObject(const std::string& name) {
    auto it = uiObjects.find(name);
    if(it != uiObjects.end()) {
        return it->second;
    }
    return nullptr;
}

UIManager* UIManager::getInstance() {
    static UIManager instance;
    return &instance;
}

unsigned int UIManager::getUIObjectCount() {
    return static_cast<unsigned int>(getInstance()->uiObjects.size());
}