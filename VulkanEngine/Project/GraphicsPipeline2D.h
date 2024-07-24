#ifndef GRAPHICSPIPELINE2D_H
#define GRAPHICSPIPELINE2D_H

#include <vulkan/vulkan.h>

#include "Scene.h"
#include "RenderPass.h"
#include "Shader.h"

class CommandPool;
class Camera;

struct GraphicsPipelineConfigs;

class GraphicsPipeline2D final
{
public:

	GraphicsPipeline2D() = default;
	~GraphicsPipeline2D() = default;

	GraphicsPipeline2D(const GraphicsPipeline2D& other) = delete;
	GraphicsPipeline2D(GraphicsPipeline2D&& other) noexcept = delete;
	GraphicsPipeline2D& operator=(const GraphicsPipeline2D& other) = delete;
	GraphicsPipeline2D& operator=(GraphicsPipeline2D&& other) noexcept = delete;

	void Initialize(const GraphicsPipelineConfigs& configs, const Camera& pCamera);
	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	void SetScene(std::vector<Model2D>&& models);

private:

	void CreateDescriptorSetLayout(VkDevice device);
	void CreateDescriptorPool(VkDevice device);
	void AllocateDescriptorSets(VkDevice device);
	void UpdateDescriptorSets(VkDevice device, const Camera& pCam);

	void CreatePipelineLayout(VkDevice device);
	void CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);

private:

	// Pipeline
	VkPipeline m_VkPipeline;
	VkPipelineLayout m_VkPipelineLayout;

	// Descriptors
	VkDescriptorSetLayout m_VkDescriptorSetLayout;
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	// Scene
	Scene2D m_Scene;

};

#endif // !GRAPHICSPIPELINE2D_H