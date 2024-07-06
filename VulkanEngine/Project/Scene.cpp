#include "Scene.h"

void Scene::Initialize(const std::string& filePath)
{
	// load models from file
}

void Scene::Initialize(std::vector<Model>&& models)
{
	m_Models = std::move(models);
}

void Scene::Destroy(VkDevice device)
{
	for (auto& model : m_Models)
	{
		model.Destroy(device);
	}
}

void Scene::UpdateBuffers(uint32_t currentFrame)
{
	for (auto& model : m_Models)
	{
		model.UpdateUniformBuffers(currentFrame);
	}
}

void Scene::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
{
	for (const auto& model : m_Models)
	{
		model.Draw(commandBuffer, pipelineLayout, descriptorSet);
	}
}

size_t Scene::GetNrOfModels() const
{
	return m_Models.size();
}

const std::vector<Model>& Scene::GetModels() const
{
	return m_Models;
}