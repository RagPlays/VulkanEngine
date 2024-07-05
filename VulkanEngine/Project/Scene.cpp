#include "Scene.h"

void Scene::Initialize(const std::string& filePath)
{
	// load models from file
}

void Scene::Initialize(std::vector<Model>&& models)
{
	m_Models = std::move(models);
}

void Scene::Draw(VkCommandBuffer commandBuffer)
{
	for (const auto& model : m_Models)
	{
		model.Draw(commandBuffer);
	}
}