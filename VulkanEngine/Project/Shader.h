#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

struct ShaderConfig
{
    std::string filePath;
    std::string entryPoint;
    VkShaderStageFlagBits stage;
};

struct ShadersConfigs
{
    ShaderConfig vertShaderConfig{};
    ShaderConfig fragShaderConfig{};
};

class Shader final
{
public:

    Shader();
    ~Shader() = default;

    void Initialize(VkDevice device, const std::string& filePath, const std::string& entryPoint, VkShaderStageFlagBits stage);
    void Initialize(VkDevice device, const ShaderConfig& shaderConfig);
    void Destroy(VkDevice device);

    VkPipelineShaderStageCreateInfo GetPipelineShaderStageInfo() const;

    static VkPipelineVertexInputStateCreateInfo GetVertex2DInputStateInfo();
    static VkPipelineVertexInputStateCreateInfo GetVertex3DInputStateInfo();
    static VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateInfo();

private:

    VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code) const;

    std::vector<char> ReadFile(const std::string& filename) const;

private:

    std::string m_FilePath;
    std::string m_EntryPoint;
    VkShaderModule m_VkShaderModule;
    VkShaderStageFlagBits m_ShaderStageFlag;

};

#endif // !SHADER_H