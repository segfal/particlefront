#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include "Renderer.cpp"

struct  Shader {
    std::string name;
    std::string vertexPath;
    std::string fragmentPath;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
};

class ShaderManager {
private:
    std::unordered_map<std::string, Shader> shaders;
    Renderer* renderer;
public:
    ShaderManager(std::vector<Shader*>& shaders) {
        renderer = Renderer::getInstance();
        for (auto& shader : shaders) {
            loadShader(shader->name, shader->vertexPath, shader->fragmentPath);
        }
    }
    ~ShaderManager();
    Shader& getShader(const std::string& name) {
        return shaders[name];
    }
    void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, int vertexBitBindings = 1, int fragmentBitBindings = 4) {
        Shader shader;
        shader.name = name;
        shader.vertexPath = vertexPath;
        shader.fragmentPath = fragmentPath;
        renderer->createDescriptorSetLayout(vertexBitBindings, fragmentBitBindings, shader.descriptorSetLayout);
        renderer->createGraphicsPipeline(shader.vertexPath, shader.fragmentPath, shader.pipeline, shader.pipelineLayout, shader.descriptorSetLayout);
        shaders[name] = shader;
    }
    static ShaderManager* getInstance() {
        static std::vector<Shader*> defaultShaders = {
            new Shader{
                .name = "pbr",
                .vertexPath = "src/assets/shaders/compiled/pbr.vert.spv",
                .fragmentPath = "src/assets/shaders/compiled/pbr.frag.spv",
            },
            new Shader{
                .name = "text",
                .vertexPath = "src/assets/shaders/compiled/text.vert.spv",
                .fragmentPath = "src/assets/shaders/compiled/text.frag.spv",
            },
            new Shader{
                .name = "ui",
                .vertexPath = "src/assets/shaders/compiled/ui.vert.spv",
                .fragmentPath = "src/assets/shaders/compiled/ui.frag.spv",
            },
        };
        static ShaderManager instance(defaultShaders);
        return &instance;
    }
};