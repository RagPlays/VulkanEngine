#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

struct ShaderConfig;

class Shader final
{
public:

    Shader();
    ~Shader() = default;

    void Initialize(VkDevice device, const ShaderConfig& shaderConfig);
    void Destroy(VkDevice device);

    VkPipelineShaderStageCreateInfo GetPipelineShaderStageInfo() const;

    static VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateInfo();

private:

    void CreateShaderModule(VkDevice device, const std::vector<char>& code);

    std::vector<char> ReadFile(const std::string& filename) const;

private:

    std::string m_FilePath;
    std::string m_EntryPoint;
    VkShaderModule m_VkShaderModule;
    VkShaderStageFlagBits m_ShaderStageFlag;

};

#endif // !SHADER_H