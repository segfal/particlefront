#pragma once
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include <vector>
#include "UIManager.cpp"
#include "Renderer.cpp"

struct Shader {
    std::string name;
    std::string vertexPath;
    std::string fragmentPath;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkPushConstantRange pushConstantRange;
    int poolMultiplier = 1;
    int vertexBitBindings = 1;
    int fragmentBitBindings = 4;
};
struct alignas(16) UIPushConstants {
    glm::vec3 color;
    uint32_t isUI;
    glm::mat4 model = glm::mat4(1.0f);
    uint32_t padding[3];
};
struct alignas(16) UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 cameraPos;
    float padding;
};

class ShaderManager {
private:
    std::unordered_map<std::string, Shader> shaders;
    Renderer* renderer;
public:
    ShaderManager(std::vector<Shader*>& shaders) {
        renderer = Renderer::getInstance();
        for (auto& shader : shaders) {
            loadShader(shader->name, shader->vertexPath, shader->fragmentPath, shader->vertexBitBindings, shader->fragmentBitBindings, shader->pushConstantRange, shader->poolMultiplier);
        }
    }
    ~ShaderManager() {
        for (auto& [name, shader] : shaders) {
            vkDestroyPipeline(renderer->device, shader.pipeline, nullptr);
            vkDestroyPipelineLayout(renderer->device, shader.pipelineLayout, nullptr);
            vkDestroyDescriptorSetLayout(renderer->device, shader.descriptorSetLayout, nullptr);
            vkDestroyDescriptorPool(renderer->device, shader.descriptorPool, nullptr);
        }
    }
    Shader* getShader(const std::string& name) {
        return &shaders[name];
    }
    void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, int vertexBitBindings, int fragmentBitBindings, VkPushConstantRange pushConstantRange = {}, int poolMultiplier = 1) {
        Shader shader;
        shader.name = name;
        shader.vertexPath = vertexPath;
        shader.fragmentPath = fragmentPath;
        renderer->createDescriptorSetLayout(vertexBitBindings, fragmentBitBindings, shader.descriptorSetLayout);
        renderer->createGraphicsPipeline(shader.vertexPath, shader.fragmentPath, shader.pipeline, shader.pipelineLayout, shader.descriptorSetLayout, shader.pushConstantRange ? &shader.pushConstantRange : nullptr);
        renderer->createDescriptorPool(shader.vertexBitBindings, shader.fragmentBitBindings, shader.descriptorPool, poolMultiplier);
        shaders[name] = shader;
    }
    static ShaderManager* getInstance() {
        static std::vector<Shader*> defaultShaders = {
            new Shader{
                .name = "pbr",
                .vertexPath = "src/assets/shaders/compiled/pbr.vert.spv",
                .fragmentPath = "src/assets/shaders/compiled/pbr.frag.spv",
                .vertexBitBindings = 1,
                .fragmentBitBindings = 4,
                .pushConstantRange = {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0,
                    .size = sizeof(UniformBufferObject),
                },
            },
            new Shader{
                .name = "ui",
                .vertexPath = "src/assets/shaders/compiled/ui.vert.spv",
                .fragmentPath = "src/assets/shaders/compiled/ui.frag.spv",
                .vertexBitBindings = 1,
                .fragmentBitBindings = 1,
                .pushConstantRange = {
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0,
                    .size = sizeof(UIPushConstants),
                },
                .poolMultiplier = UIManager::getUIObjectCount() > 0 ? UIManager::getUIObjectCount() : 1,
            },
        };
        static ShaderManager instance(defaultShaders);
        return &instance;
    }
};