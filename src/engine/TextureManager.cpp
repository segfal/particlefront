#include "TextureManager.h"
#include "Renderer.h"
#include <filesystem>
#include <stb/stb_image.h>
#include "../utils.h"

TextureManager::TextureManager() {
    renderer = Renderer::getInstance();
    findAllTextures();
    prepareTextureAtlas();
}
TextureManager::~TextureManager() {
    shutdown();
}
void TextureManager::findAllTextures(std::string path, std::string prevName) {
    namespace fs = std::filesystem;
    fs::path searchPath = resolvePath(path);
    std::error_code ec;
    if ((!fs::exists(searchPath, ec) || !fs::is_directory(searchPath, ec)) && prevName.empty()) {
        searchPath = resolvePath("src/assets/textures/");
    }
    if (!fs::exists(searchPath, ec) || !fs::is_directory(searchPath, ec)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(searchPath)) {
        std::string name = entry.path().stem().string();
        if (entry.path().extension() == ".png") {
            textureAtlas[prevName + name] = Image{};
            textureAtlas[prevName + name].path = entry.path().string();
        } else if (entry.is_directory()) {
            findAllTextures(entry.path().string(), prevName + name + "_");
        }
    }
}
void TextureManager::prepareTextureAtlas() {
        std::vector<std::pair<std::string, ImageData>> loadedImages;
        #pragma omp parallel for
        for (int i = 0; i < static_cast<int>(textureAtlas.size()); ++i) {
            auto it = std::next(textureAtlas.begin(), i);
            stbi_set_flip_vertically_on_load(true);
            int texWidth, texHeight, texChannels;
            stbi_uc* pixels = stbi_load(it->second.path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            #pragma omp critical
            {
                loadedImages.push_back({it->first, {pixels, texWidth, texHeight}});
            }
        }
        for (auto& [name, imageData] : loadedImages) {
            Image& texture = textureAtlas[name];
            stbi_uc* pixels = imageData.pixels;
            if (!pixels || imageData.width <= 0 || imageData.height <= 0) continue;
            VkDeviceSize imageSize = static_cast<VkDeviceSize>(imageData.width) * static_cast<VkDeviceSize>(imageData.height) * 4;
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            renderer->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            void* data;
            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            vkMapMemory(renderer->device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, (size_t) imageSize);
            vkUnmapMemory(renderer->device, stagingBufferMemory);
            stbi_image_free(pixels);
            renderer->createImage(imageData.width, imageData.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            renderer->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            renderer->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(imageData.width), static_cast<uint32_t>(imageData.height));
            renderer->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            vkDestroyBuffer(renderer->device, stagingBuffer, nullptr);
            vkFreeMemory(renderer->device, stagingBufferMemory, nullptr);
            texture.image = textureImage;
            texture.imageMemory = textureImageMemory;
            renderer->createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB, textureImage, texture.imageView);
        }
}
Image* TextureManager::getTexture(const std::string& name) {
        auto it = textureAtlas.find(name);
        if(it != textureAtlas.end()) {
            return &it->second;
        }
        return nullptr;
}
void TextureManager::shutdown() {
    if (!renderer || renderer->device == VK_NULL_HANDLE) {
        textureAtlas.clear();
        return;
    }
    for(auto& [name, texture] : textureAtlas) {
        if (texture.imageSampler) {
            vkDestroySampler(renderer->device, texture.imageSampler, nullptr);
            texture.imageSampler = VK_NULL_HANDLE;
        }
        if (texture.imageView) {
            vkDestroyImageView(renderer->device, texture.imageView, nullptr);
            texture.imageView = VK_NULL_HANDLE;
        }
        if (texture.image) {
            vkDestroyImage(renderer->device, texture.image, nullptr);
            texture.image = VK_NULL_HANDLE;
        }
        if (texture.imageMemory) {
            vkFreeMemory(renderer->device, texture.imageMemory, nullptr);
            texture.imageMemory = VK_NULL_HANDLE;
        }
    }
    textureAtlas.clear();
}
TextureManager* TextureManager::getInstance() {
    static TextureManager instance;
    return &instance;
}