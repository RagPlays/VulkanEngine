#include "Scene.h"

// SCENE 2D //

void Scene2D::Initialize(const std::string& filePath)
{
	// load models from file
}

void Scene2D::Initialize(std::vector<Model2D>&& models)
{
	m_Models = std::move(models);
}

void Scene2D::Destroy(VkDevice device)
{
	for (auto& model : m_Models)
	{
		model.Destroy(device);
	}
	m_Models.clear();
}

void Scene2D::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	for (const auto& model : m_Models)
	{
		model.Draw(commandBuffer, pipelineLayout);
	}
}

size_t Scene2D::GetNrOfModels() const
{
	return m_Models.size();
}

const std::vector<Model2D>& Scene2D::GetModels() const
{
	return m_Models;
}

// SCENE 3D //

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
	m_Models.clear();
}

void Scene3D::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	for (const auto& model : m_Models)
	{
		model.Draw(commandBuffer, pipelineLayout);
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