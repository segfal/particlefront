#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>
#include "Image.h"

class Renderer;

class TextureManager {
private:
    Renderer* renderer;
    std::unordered_map<std::string, Image> textureAtlas;

public:
    TextureManager();
    ~TextureManager();

    void findAllTextures(std::string path = "src/textures/ui/", std::string prevName = "");

    struct ImageData { unsigned char* pixels; int width; int height; };
    void prepareTextureAtlas();

    Image* getTexture(const std::string& name);
    void registerTexture(const std::string& name, const Image& texture);
    void shutdown();
    static TextureManager* getInstance();
};
