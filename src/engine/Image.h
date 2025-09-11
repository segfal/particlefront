#pragma once
#include <vulkan/vulkan.h>

class Image {
public:
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;
    VkSampler imageSampler;
    VkFormat format;
};