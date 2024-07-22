#ifndef GRAPHICSPIPELINE3D_H
#define GRAPHICSPIPELINE3D_H

#include "Scene.h"

class CommandPool;

struct ShaderConfig;
struct ShadersConfigs;

struct GraphicsPipelineConfigs;

class GraphicsPipeline3D final
{
public:

	GraphicsPipeline3D() = default;
	~GraphicsPipeline3D() = default;

	GraphicsPipeline3D(const GraphicsPipeline3D& other) = delete;
	GraphicsPipeline3D(GraphicsPipeline3D&& other) noexcept = delete;
	GraphicsPipeline3D& operator=(const GraphicsPipeline3D& other) = delete;
	GraphicsPipeline3D& operator=(GraphicsPipeline3D&& other) noexcept = delete;

	void Initialize(const GraphicsPipelineConfigs& configs);
	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet) const;

	void SetScene(std::vector<Model3D>&& models);

	size_t GetNrOfModels() const;
	const VkDescriptorSetLayout& GetDescriptorSetLayout() const;

private:

	void CreateDescriptorSetLayout(VkDevice device);

	void CreatePipelineLayout(VkDevice device);
	void CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass);

private:

	// Pipeline
	VkPipeline m_VkPipeline;
	VkPipelineLayout m_VkPipelineLayout;

	// Descritor
	VkDescriptorSetLayout m_VkDescriptorSetLayout;

	// Scene
	Scene3D m_Scene;
};

#endif // !GRAPHICSPIPELINE3D_H