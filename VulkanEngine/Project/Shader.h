#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class Shader final
{
public:

    explicit Shader(VkDevice device, const std::string& filePath, const std::string& entryPoint, VkShaderStageFlagBits stage);
    ~Shader();

    VkPipelineShaderStageCreateInfo GetPipelineShaderStageInfo() const;

    static VkPipelineVertexInputStateCreateInfo GetVertexInputStateInfo();
    static VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyStateInfo();

private:

    VkShaderModule CreateShaderModule(const std::vector<char>& code) const;

private:

    const std::string m_FilePath;
    const std::string m_EntryPoint;
    const VkDevice m_VkDevice;
    VkShaderModule m_VkShaderModule;
    VkShaderStageFlagBits m_ShaderStageFlag;

};

#endif // !SHADER_H