#include "UIObject.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include <iostream>
#include <vector>

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
    for (auto &entry : children) {
        if (std::holds_alternative<UIObject*>(entry.second)) {
            if (UIObject* child = std::get<UIObject*>(entry.second)) {
                child->loadTexture();
            }
        }
    }
}
