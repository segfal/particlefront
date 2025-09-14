#ifndef PARTICLEFRONT_ENGINE_RENDERER_H
#define PARTICLEFRONT_ENGINE_RENDERER_H

#include <cstdint>
#include <memory>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace engine {

class Mesh;
class Shader;
class Texture;
class Camera;

class Renderer {
public:
    struct InitInfo {
        int width = 1280;
        int height = 720;
        bool vsync = true;
        bool debugContext = false;
    };
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer();
    ~Renderer();

    static Renderer* getInstance();

    void createTextureImage(int width, int height, unsigned char* imageBuffer, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkFormat format);

    VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags);
    
    void createTextureImageView(VkFormat textureFormat, VkImage textureImage, VkImageView &textureImageView);

    void overwriteUIManager(class UIManager* uiManager);

    void run();
};
}

#endif