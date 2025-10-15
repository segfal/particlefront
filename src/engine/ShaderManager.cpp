#include "ShaderManager.h"
#include "Renderer.h"

ShaderManager::ShaderManager(std::vector<Shader*>& shaders) {
    renderer = Renderer::getInstance();
    for (auto& shader : shaders) {
        loadShader(shader->name, shader->vertexPath, shader->fragmentPath, shader->vertexBitBindings, shader->fragmentBitBindings, shader->pushConstantRange, shader->poolMultiplier);
    }
}
ShaderManager::~ShaderManager() {
    shutdown();
}
Shader* ShaderManager::getShader(const std::string& name) {
    return &shaders[name];
}
void ShaderManager::shutdown() {
    if (!renderer || renderer->device == VK_NULL_HANDLE) {
        shaders.clear();
        return;
    }
    for (auto& [name, shader] : shaders) {
        if (shader.pipeline) {
            vkDestroyPipeline(renderer->device, shader.pipeline, nullptr);
            shader.pipeline = VK_NULL_HANDLE;
        }
        if (shader.pipelineLayout) {
            vkDestroyPipelineLayout(renderer->device, shader.pipelineLayout, nullptr);
            shader.pipelineLayout = VK_NULL_HANDLE;
        }
        if (shader.descriptorSetLayout) {
            vkDestroyDescriptorSetLayout(renderer->device, shader.descriptorSetLayout, nullptr);
            shader.descriptorSetLayout = VK_NULL_HANDLE;
        }
        if (shader.descriptorPool) {
            vkDestroyDescriptorPool(renderer->device, shader.descriptorPool, nullptr);
            shader.descriptorPool = VK_NULL_HANDLE;
        }
    }
    shaders.clear();
}
void ShaderManager::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, int vertexBitBindings, int fragmentBitBindings, VkPushConstantRange pushConstantRange, int poolMultiplier) {
    Shader shader = {
        .name = name,
        .vertexPath = vertexPath,
        .fragmentPath = fragmentPath,
        .pushConstantRange = pushConstantRange,
        .poolMultiplier = poolMultiplier,
        .vertexBitBindings = vertexBitBindings,
        .fragmentBitBindings = fragmentBitBindings,
    };

    renderer->createDescriptorSetLayout(shader.vertexBitBindings, shader.fragmentBitBindings, shader.descriptorSetLayout);
    VkPushConstantRange* pPCR = (shader.pushConstantRange.size > 0) ? &shader.pushConstantRange : nullptr;
    const bool enableDepth = (name != "ui");
    const bool useTextVertex = (name == "ui");
    renderer->createGraphicsPipeline(shader.vertexPath, shader.fragmentPath, shader.pipeline, shader.pipelineLayout, shader.descriptorSetLayout, pPCR, enableDepth, useTextVertex);
    renderer->createDescriptorPool(shader.vertexBitBindings, shader.fragmentBitBindings, shader.descriptorPool, shader.poolMultiplier);
    shaders[name] = shader;
}
ShaderManager* ShaderManager::getInstance() {
    static std::vector<Shader*> defaultShaders = {
        new Shader{
            .name = "pbr",
            .vertexPath = "src/assets/shaders/compiled/pbr.vert.spv",
            .fragmentPath = "src/assets/shaders/compiled/pbr.frag.spv",
            .pushConstantRange = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(UniformBufferObject),
            },
            .poolMultiplier = 256,
            .vertexBitBindings = 1,
            .fragmentBitBindings = 4,
        },
        new Shader{
            .name = "ui",
            .vertexPath = "src/assets/shaders/compiled/ui.vert.spv",
            .fragmentPath = "src/assets/shaders/compiled/ui.frag.spv",
            .pushConstantRange = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(UIPushConstants),
            },
            .poolMultiplier = 256,
            .vertexBitBindings = 0,
            .fragmentBitBindings = 1,
        },
    };
    static ShaderManager instance(defaultShaders);
    return &instance;
}