#include "ShaderManager.h"
#include "Renderer.h"

ShaderManager::ShaderManager(std::vector<Shader*>& shaders) {
    renderer = Renderer::getInstance();
    for (auto& shader : shaders) {
        loadShader(shader->name, shader->vertexPath, shader->fragmentPath, shader->vertexBitBindings, shader->fragmentBitBindings, shader->pushConstantRange, shader->poolMultiplier);
    }
}
ShaderManager::~ShaderManager() {
    for (auto& [name, shader] : shaders) {
        vkDestroyPipeline(renderer->device, shader.pipeline, nullptr);
        vkDestroyPipelineLayout(renderer->device, shader.pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(renderer->device, shader.descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(renderer->device, shader.descriptorPool, nullptr);
    }
}
Shader* ShaderManager::getShader(const std::string& name) {
    return &shaders[name];
}
void ShaderManager::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, int vertexBitBindings, int fragmentBitBindings, VkPushConstantRange pushConstantRange, int poolMultiplier) {
    Shader shader = {
        .name = name,
        .vertexPath = vertexPath,
        .fragmentPath = fragmentPath,
        .vertexBitBindings = vertexBitBindings,
        .fragmentBitBindings = fragmentBitBindings,
        .pushConstantRange = pushConstantRange,
        .poolMultiplier = poolMultiplier,
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
            .vertexBitBindings = 0,
            .fragmentBitBindings = 1,
            .pushConstantRange = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(UIPushConstants),
            },
            .poolMultiplier = 256,
        },
    };
    static ShaderManager instance(defaultShaders);
    return &instance;
}