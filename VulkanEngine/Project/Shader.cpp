#include "Shader.h"
#include "VulkanUtils.h"
#include "Vertex.h"

Shader::Shader()
    : m_FilePath{}
    , m_EntryPoint{}
    , m_VkShaderModule{ VK_NULL_HANDLE }
    , m_ShaderStageFlag{}
{
}

void Shader::Initialize(VkDevice device, const std::string& filePath, const std::string& entryPoint, VkShaderStageFlagBits stage)
{
    m_FilePath = filePath;
    m_EntryPoint = entryPoint;
    m_ShaderStageFlag = stage;
    std::vector<char> code{ ReadFile(m_FilePath) };
    m_VkShaderModule = CreateShaderModule(device, code);
}

void Shader::Initialize(VkDevice device, const ShaderConfig& shaderConfig)
{
    m_FilePath = shaderConfig.filePath;
    m_EntryPoint = shaderConfig.entryPoint;
    m_ShaderStageFlag = shaderConfig.stage;
    std::vector<char> code{ ReadFile(m_FilePath) };
    m_VkShaderModule = CreateShaderModule(device, code);
}

void Shader::Destroy(VkDevice device)
{
    vkDestroyShaderModule(device, m_VkShaderModule, nullptr);
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

VkPipelineVertexInputStateCreateInfo Shader::GetVertex2DInputStateInfo()
{
    auto bindingDescription{ new VkVertexInputBindingDescription{ Vertex2D::GetBindingDescription() } };
    auto attributeDescriptions{ new std::array<VkVertexInputAttributeDescription, 2>{ Vertex2D::GetAttributeDescriptions() } };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions->size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions->data();
    return vertexInputInfo;
}

VkPipelineVertexInputStateCreateInfo Shader::GetVertex3DInputStateInfo()
{
    auto bindingDescription{ new VkVertexInputBindingDescription{ Vertex3D::GetBindingDescription() } };
    auto attributeDescriptions{ new std::array<VkVertexInputAttributeDescription, 3>{ Vertex3D::GetAttributeDescriptions() } };

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

VkShaderModule Shader::CreateShaderModule(VkDevice device, const std::vector<char>& code) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule{};
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> Shader::ReadFile(const std::string& filename) const
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}