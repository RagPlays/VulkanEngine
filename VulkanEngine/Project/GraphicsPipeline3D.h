#ifndef GRAPHICSPIPELINE3D_H
#define GRAPHICSPIPELINE3D_H

#include "Scene.h"

class CommandPool;

struct ShaderConfig;

class GraphicsPipeline3D final
{
public:

	GraphicsPipeline3D() = default;
	~GraphicsPipeline3D() = default;

	void Initialize(VkDevice device, const ShaderConfig& vertSConf, const ShaderConfig& fragSConf, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);
	void InitScene(VkDevice device, VkPhysicalDevice phyDevice, VkQueue graphxQueue, const CommandPool& cmndPl);

	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, uint32_t currentFrame) const;

	size_t GetNrOfModels() const;
	const VkDescriptorSetLayout& GetDescriptorSetLayout() const;

private:

	void CreateDescriptorSetLayout(VkDevice device);
	void CreatePipelineLayout(VkDevice device);
	void CreatePipeline(VkDevice device, const ShaderConfig& vertSConf, const ShaderConfig& fragSConf, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);

private:

	// Pipeline
	VkPipeline m_VkPipeline;
	VkPipelineLayout m_VkPipelineLayout;

	VkDescriptorSetLayout m_VkDescriptorSetLayout;

	// Scene
	Scene3D m_Scene;
};

#endif // !GRAPHICSPIPELINE3D_H