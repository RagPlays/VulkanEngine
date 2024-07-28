#include <fstream>

#include <nlohmann/json.hpp>

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

void Scene3DIR::Initialize(const VulkanInstance& instance, const CommandPool& commandPool, const std::string& filePath)
{
	if (!m_Models.empty()) throw std::runtime_error{ "Scene already initialized!" };

	// load models from file
	if (std::ifstream file{ filePath }; file.is_open())
	{
		nlohmann::json sceneData{};
		file >> sceneData;

		for (const auto& modelData : sceneData["models"])
		{
			const std::string modelFilePath{ modelData["file"] };
			std::vector<glm::vec3> positions{};
			positions.reserve(modelData["instances"].size());

			for (const auto& positionData : modelData["instances"])
			{
				positions.emplace_back
				(
					glm::vec3
					{
						positionData["position"][0],
						positionData["position"][1],
						positionData["position"][2]
					}
				);
			}

			Model3DIR model{};
			model.Initialize(instance, commandPool, modelFilePath, static_cast<uint32_t>(positions.size()));

			for (size_t modelInstanceIdx{}; modelInstanceIdx < positions.size(); ++modelInstanceIdx)
			{
				const uint32_t idx{ static_cast<uint32_t>(modelInstanceIdx) };
				model.SetPosition(idx, positions[modelInstanceIdx]);
				model.SetRotation(idx, Transform3D::GetRandomRotationVec());
				model.SetScale(idx, 1.f);
			}

			m_Models.push_back(std::move(model));
		}
	}
	else throw std::runtime_error{ "Failed to open scene file: " + filePath };
}

void Scene3DIR::Initialize(std::vector<Model3DIR>&& models)
{
	if (!m_Models.empty()) throw std::runtime_error{ "Scene already initialized!" };

	m_Models = std::move(models);
}

void Scene3DIR::Destroy(VkDevice device)
{
	for (auto& model : m_Models)
	{
		model.Destroy(device);
	}
	m_Models.clear();
}

void Scene3DIR::Update(VkDevice device)
{
	for (auto& model : m_Models)
	{
		model.Update(device);
	}
}

void Scene3DIR::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	for (const auto& model : m_Models)
	{
		model.Draw(commandBuffer, pipelineLayout);
	}
}