#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <optional>

struct GLFWwindow;
class UIManager;
class FontManager;
class SceneManager;
class EntityManager;
class ModelManager;
class ShaderManager;
class TextureManager;
class Image;
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Renderer {
public:
    VkDevice device;

    Renderer();
    ~Renderer();
    static Renderer* getInstance();

    void run();
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createTextureImage(int width, int height, unsigned char* imageBuffer, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkFormat format);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createTextureImageView(VkFormat textureFormat, VkImage textureImage, VkImageView &textureImageView);
    VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createDescriptorSetLayout(int vertexBitBindings, int fragmentBitBindings, VkDescriptorSetLayout& descriptorSetLayout);
    void createDescriptorPool(int vertexBitBindings, int fragmentBitBindings, VkDescriptorPool &descriptorPool, int multiplier = 1);
    std::vector<VkDescriptorSet> createDescriptorSets(VkDescriptorPool pool, VkDescriptorSetLayout& descriptorSetLayout, int vertexBindingCount, int fragmentBindingCount, std::vector<Image*>& textures, std::vector<VkBuffer>& uniformBuffers);
    void createGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout& descriptorSetLayout, VkPushConstantRange* pushConstantRange = nullptr, bool enableDepth = true, bool useTextVertex = false);
    void createCommandBuffers();
    void createSyncObjects();
    void createTextureSampler();
    void createTextureSampler(VkSampler &sampler, uint32_t mipLevels = 1);

    VkDevice getDevice() const { return device; }
    ShaderManager* getShaderManager() const;

private:
    void initWindow();
    void cleanup();
    void initVulkan();
    void mainLoop();
    void updateUniformBuffer(uint32_t currentFrame);
    void drawFrame();

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void recreateSwapChain();
    void cleanupSwapChain();
    void createImageViews();
    void createRenderPass();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void createCommandPool();
    void createQuadBuffers();
    void setupUI();
    void renderUI(VkCommandBuffer commandBuffer);
    void renderEntities(VkCommandBuffer commandBuffer);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createColorResources();
    void createDepthResources();
    void createFramebuffers();
    int rateDeviceSuitability(VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool hasStencilComponent(VkFormat format);
    bool hasDeviceExtension(VkPhysicalDevice dev, const char* extName);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static void processInput(GLFWwindow* window);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

    GLFWwindow* window = nullptr;
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkSurfaceKHR surface{};
    VkSwapchainKHR swapChain{};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};
    VkCommandPool commandPool{};
    VkSampler textureSampler{};
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    VkImage colorImage{};
    VkDeviceMemory colorImageMemory{};
    VkImageView colorImageView{};
    VkImage depthImage{};
    VkDeviceMemory depthImageMemory{};
    VkImageView depthImageView{};
    VkBuffer quadVertexBuffer{};
    VkDeviceMemory quadVertexBufferMemory{};
    VkBuffer quadIndexBuffer{};
    VkDeviceMemory quadIndexBufferMemory{};
    UIManager* uiManager = nullptr;
    FontManager* fontManager = nullptr;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    ShaderManager* shaderManager = nullptr;
    SceneManager* sceneManager = nullptr;
    TextureManager* textureManager = nullptr;
    ModelManager* modelManager = nullptr;
    EntityManager* entityManager = nullptr;
    bool framebufferResized = false;
    bool firstMouse = true;
    float uiScale = 1.0f;
    float textScale = 1.0f;
    float textSizeScale = 1.0f;
};
