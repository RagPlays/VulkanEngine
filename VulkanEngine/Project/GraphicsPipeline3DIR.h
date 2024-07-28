#ifndef GRAPHICSPIPELINE3DIR_H
#define GRAPHICSPIPELINE3DIR_H

#include <vector>

#include <vulkan/vulkan.h>

#include "Scene.h"

class VulkanInstance;
class Texture;
class Camera;

struct GraphicsPipelineConfigs;
struct ShadersConfigs;

class GraphicsPipeline3DIR final // (IR for Instance Rendering)
{
public:

	GraphicsPipeline3DIR() = default;
	~GraphicsPipeline3DIR() = default;

	void Initialize(const GraphicsPipelineConfigs& configs, const Texture& tex, const Camera& cam);
	void Destory(VkDevice device);

	void Update(VkDevice device);
	void Draw(VkCommandBuffer commandBuffer, uint32_t currentFrame) const;

	void SetScene(Scene3DIR&& scene);
	void SetScene(std::vector<Model3DIR>&& models);

private:

	void CreateDescriptorSetLayout(VkDevice device);
	void CreateDescriptorPool(VkDevice device);
	void AllocateDescriptorSets(VkDevice device);
	void UpdateDescriptorSets(VkDevice device, const Texture& tex, const Camera& cam);

	void CreatePipelineLayout(VkDevice device);
	void CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);

private:

	// Pipeline
	VkPipeline m_VkPipeline;
	VkPipelineLayout m_VkPipelineLayout;

	// Descritor
	VkDescriptorSetLayout m_VkDescriptorSetLayout;
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	// Scene
	Scene3DIR m_Scene;

};

#endif // !GRAPHICSPIPELINE3DIR_H