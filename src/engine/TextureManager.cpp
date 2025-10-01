#include "Renderer.cpp"
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>

struct Texture {
    std::string path;
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;
    int width;
    int height;
};

class TextureManager {
private:
    Renderer* renderer;
    std::unordered_map<std::string, Texture> textures;
    unordered_map<std::string, Texture> textureAtlas;
public:
    TextureManager() {
        renderer = Renderer::getInstance();
        findAllTextures();
        prepareTextureAtlas();
    }
    ~TextureManager() {
        for(auto& [name, texture] : textures) {
            vkDestroySampler(renderer->device, texture.sampler, nullptr);
            vkDestroyImageView(renderer->device, texture.imageView, nullptr);
            vkDestroyImage(renderer->device, texture.image, nullptr);
            vkFreeMemory(renderer->device, texture.imageMemory, nullptr);
        }
    }
    void findAllTextures(std::string path = "src/textures/ui/", std::string prevName = "") {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::string name = entry.path().stem().string();
            if (entry.path().extension() == ".png") {
                textureAtlas[name] = Texture{};
                textureAtlas[name].path = entry.path().string();
            } else if (entry.is_directory()) {
                findAllTextures(entry.path().string() + "/", prevName + name + "_");
            }
        }
    }
    void prepareTextureAtlas() {
        std::vector<std::pair<std::string, ImageData>> loadedImages;
        #pragma omp parallel for
        for (int i = 0; i < textureAtlas.size(); ++i) {
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
            Texture& texture = textureAtlas[name];
            stbi_uc* pixels = imageData.pixels;
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            renderer->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
            void* data;
            VkImage textureImage;
            VkDeviceMemory textureImageMemory;
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, (size_t) imageSize);
            vkUnmapMemory(device, stagingBufferMemory);
            stbi_image_free(pixels);
            renderer->createImage(texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
            renderer->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            renderer->copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
            renderer->transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            texture.image = textureImage;
            texture.imageMemory = textureImageMemory;
            renderer->createTextureImageView(VK_FORMAT_R8G8B8A8_SRGB, textureImage, texture.imageView);
        }
    }
    static TextureManager* getInstance() {
        static TextureManager instance;
        return &instance;
    }
};