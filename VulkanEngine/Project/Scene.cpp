#include "Scene.h"

void Scene3D::Initialize(const std::string& filePath)
{
	// load models from file
}

void Scene3D::Initialize(std::vector<Model3D>&& models)
{
	m_Models = std::move(models);
}

void Scene3D::Destroy(VkDevice device)
{
	for (auto& model : m_Models)
	{
		model.Destroy(device);
	}
}

void Scene3D::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) const
{
	for (const auto& model : m_Models)
	{
		model.Draw(commandBuffer, pipelineLayout, currentFrame);
	}
}

size_t Scene3D::GetNrOfModels() const
{
	return m_Models.size();
}

const std::vector<Model3D>& Scene3D::GetModels() const
{
	return m_Models;
}