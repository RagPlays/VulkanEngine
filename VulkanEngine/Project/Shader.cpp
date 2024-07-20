#include <memory>

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
    if (code.empty())
    {
        throw std::runtime_error{ "Shader code is empty, failed to read file: " + m_FilePath };
    }

    m_VkShaderModule = CreateShaderModule(device, code);
}

void Shader::Initialize(VkDevice device, const ShaderConfig& shaderConfig)
{
    m_FilePath = shaderConfig.filePath;
    m_EntryPoint = shaderConfig.entryPoint;
    m_ShaderStageFlag = shaderConfig.stage;

    std::vector<char> code{ ReadFile(m_FilePath) };
    if (code.empty())
    {
        throw std::runtime_error{ "Shader code is empty, failed to read file: " + m_FilePath };
    }

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

    if (!shaderStageInfo.module)
    {
        throw std::runtime_error("Shader modules not created correctly!");
    }

    return shaderStageInfo;
}

VkPipelineInputAssemblyStateCreateInfo Shader::GetInputAssemblyStateInfo()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // VK_PRIMITIVE_TOPOLOGY_LINE_LIST -> for 2D line Lib
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