#include "UIObject.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include <iostream>
#include <vector>

UIObject::~UIObject() {
    for (auto& entry : children) {
        delete entry.second;
    }
    children.clear();
}

void UIObject::addChild(UIObject* child) {
    if (!child) {
        return;
    }
    children[child->getName()] = child;
}

void UIObject::removeChild(UIObject* child) {
    if (!child) {
        return;
    }
    auto it = children.find(child->getName());
    if (it != children.end()) {
        delete it->second;
        children.erase(it);
    }
}

void UIObject::loadTexture() {
    if (!texture.empty()) {
        TextureManager* texMgr = TextureManager::getInstance();
        Image* img = texMgr->getTexture(texture);
        if (!img) {
            std::cerr << "Texture " << texture << " not found!" << std::endl;
        } else {
            Renderer* renderer = Renderer::getInstance();
            Shader* uiShader = renderer->getShaderManager()->getShader("ui");
            std::vector<Image*> textures = {img};
            std::vector<VkBuffer> buffers;
            descriptorSets = renderer->createDescriptorSets(
                uiShader->descriptorPool,
                uiShader->descriptorSetLayout,
                uiShader->vertexBitBindings,
                uiShader->fragmentBitBindings,
                textures,
                buffers
            );
        }
    }
    for (auto& entry : children) {
        if (entry.second) {
            entry.second->loadTexture();
        }
    }
}
