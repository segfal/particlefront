#pragma once
#include <vulkan/vulkan.h>
#include <string>

class Image {
public:
    Image() = default;
    ~Image() = default;
    std::string path;
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkSampler imageSampler = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    int width = 0;
    int height = 0;
};