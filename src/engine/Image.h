#pragma once
#include <vulkan/vulkan.h>

class Image {
public:
    Image() = default;
    ~Image() = default;
    std::string path;
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;
    VkSampler imageSampler;
    VkFormat format;
    int width;
    int height;
};