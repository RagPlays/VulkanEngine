#include "GraphicsPipeline2D.h"

#include "Shader.h"


void GraphicsPipeline2D::Initialize(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass)
{
	CreatePipelineLayout(device);
	CreatePipeline(device, shaderConfigs, swapchainExtent, renderPass);
}

void GraphicsPipeline2D::Destroy(VkDevice device)
{
}

void GraphicsPipeline2D::CreatePipelineLayout(VkDevice device)
{
}

void GraphicsPipeline2D::CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass)
{
}
