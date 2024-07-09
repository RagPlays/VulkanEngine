#ifndef GRAPHICSPIPELINE2D_H
#define GRAPHICSPIPELINE2D_H

#include <vulkan/vulkan.h>

#include "Scene.h"

class CommandPool;

struct ShadersConfigs;

class GraphicsPipeline2D final
{
public:

	GraphicsPipeline2D() = default;
	~GraphicsPipeline2D() = default;

	GraphicsPipeline2D(const GraphicsPipeline2D& other) = delete;
	GraphicsPipeline2D(GraphicsPipeline2D&& other) noexcept = delete;
	GraphicsPipeline2D& operator=(const GraphicsPipeline2D& other) = delete;
	GraphicsPipeline2D& operator=(GraphicsPipeline2D&& other) noexcept = delete;

	void Initialize(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
	void Destroy(VkDevice device);

private:

	void CreatePipelineLayout(VkDevice device);
	void CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);

private:

	Scene2D m_Scene;

};

#endif // !GRAPHICSPIPELINE2D_H