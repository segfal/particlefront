#include <ShaderManager.h>
#include <Renderer.h>

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
    const bool isUI = (name == "ui");
    const bool isSkybox = (name == "skybox");
    const bool isGBuffer = (name == "gbuffer");
    const bool isDeferredLighting = (name == "lighting");
    const bool isComposite = (name == "composite");
    const bool enableDepth = !isUI && !isDeferredLighting && !isComposite;
    const bool useTextVertex = isUI || isDeferredLighting || isComposite;
    const VkCullModeFlags cullMode = isSkybox ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
    const bool depthWrite = isSkybox ? false : enableDepth;
    const VkCompareOp depthCompare = isSkybox ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_LESS;
    const VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkRenderPass renderPassToUse = VK_NULL_HANDLE;
    uint32_t colorAttachmentCount = 1;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    bool noVertexInput = false;
    
    if (isGBuffer) {
        renderPassToUse = renderer->getGBufferRenderPass();
        colorAttachmentCount = 3;
    } else if (isDeferredLighting) {
        renderPassToUse = renderer->getLightingRenderPass();
        noVertexInput = true;
    } else if (isComposite) {
        renderPassToUse = renderer->getCompositeRenderPass();
        noVertexInput = true;
    } else if (isUI) {
        renderPassToUse = renderer->getCompositeRenderPass();
    } else if (isSkybox) {
        renderPassToUse = renderer->getGBufferRenderPass();
        colorAttachmentCount = 3;
    }
    renderer->createGraphicsPipeline(shader.vertexPath, shader.fragmentPath, shader.pipeline, shader.pipelineLayout, shader.descriptorSetLayout, pPCR, enableDepth, useTextVertex, cullMode, frontFace, depthWrite, depthCompare, renderPassToUse, colorAttachmentCount, sampleCount, noVertexInput);
    renderer->createDescriptorPool(shader.vertexBitBindings, shader.fragmentBitBindings, shader.descriptorPool, shader.poolMultiplier);
    shaders[name] = shader;
}
ShaderManager* ShaderManager::getInstance() {
    static std::vector<Shader*> defaultShaders = {
        new Shader{
            .name = "gbuffer",
            .vertexPath = "src/assets/shaders/compiled/gbuffer.vert.spv",
            .fragmentPath = "src/assets/shaders/compiled/gbuffer.frag.spv",
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
            .name = "lighting",
            .vertexPath = "src/assets/shaders/compiled/lighting.vert.spv",
            .fragmentPath = "src/assets/shaders/compiled/lighting.frag.spv",
            .pushConstantRange = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(LightingPushConstants),
            },
            .poolMultiplier = 4,
            .vertexBitBindings = 0,
            .fragmentBitBindings = 4,
        },
        new Shader{
            .name = "composite",
            .vertexPath = "src/assets/shaders/compiled/composite.vert.spv",
            .fragmentPath = "src/assets/shaders/compiled/composite.frag.spv",
            .pushConstantRange = {},
            .poolMultiplier = 4,
            .vertexBitBindings = 0,
            .fragmentBitBindings = 2,
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
        new Shader{
            .name = "skybox",
            .vertexPath = "src/assets/shaders/compiled/skybox.vert.spv",
            .fragmentPath = "src/assets/shaders/compiled/skybox.frag.spv",
            .pushConstantRange = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(UniformBufferObject),
            },
            .poolMultiplier = 64,
            .vertexBitBindings = 1,
            .fragmentBitBindings = 1,
        },
    };
    static ShaderManager instance(defaultShaders);
    return &instance;
}