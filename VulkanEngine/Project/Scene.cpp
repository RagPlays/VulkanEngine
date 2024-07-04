#include "Scene.h"

void Scene::Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer, VkDescriptorSet discriptorSet)
{
	for (const auto& model : m_Models)
	{
		model.Draw(pipelineLayout, commandBuffer, discriptorSet);
	}
}