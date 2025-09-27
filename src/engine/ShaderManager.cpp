#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>
#include "Renderer.h"

struct  Shader {
    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
};

class ShaderManager {
private:
    std::unordered_map<std::string, Shader> shaders;
    engine::Renderer* renderer;
public:
    ShaderManager() {
        renderer = engine::Renderer::getInstance();
    }
    ~ShaderManager();
    Shader& getShader(const std::string& name) {
        return shaders[name];
    }
    void loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
};