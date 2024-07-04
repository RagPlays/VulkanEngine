#include "Shader.h"
#include "VulkanUtils.h"
#include "VulkanStructs.h"

Shader::Shader(VkDevice device, const std::string& filePath, const std::string& entryPoint, VkShaderStageFlagBits stage)
    : m_FilePath{ filePath }
    , m_EntryPoint{ entryPoint }
    , m_VkDevice{ device }
    , m_VkShaderModule{ VK_NULL_HANDLE }
    , m_ShaderStageFlag{ stage }
{
    std::vector<char> code{ ReadFile(m_FilePath) };
    m_VkShaderModule = CreateShaderModule(code);
}

Shader::~Shader()
{
    vkDestroyShaderModule(m_VkDevice, m_VkShaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::GetPipelineShaderStageInfo() const
{
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = m_ShaderStageFlag;
    shaderStageInfo.module = m_VkShaderModule;
    shaderStageInfo.pName = m_EntryPoint.c_str();

    return shaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo Shader::GetVertexInputStateInfo()
{
    auto bindingDescription{ new VkVertexInputBindingDescription{ Vertex::GetBindingDescription() } };
    auto attributeDescriptions{ new std::array<VkVertexInputAttributeDescription, 3>{ Vertex::GetAttributeDescriptions() } };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions->size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions->data();
    return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo Shader::GetInputAssemblyStateInfo()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // VK_PRIMITIVE_TOPOLOGY_LINE_LIST -> for 2D Lib
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    return inputAssembly;
}

VkShaderModule Shader::CreateShaderModule(const std::vector<char>& code) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_VkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}