#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Model.h"

class Camera;
class VulkanInstance;
class CommandPool;

class Scene2D final
{
public:

	Scene2D() = default;
	~Scene2D() = default;

	void Initialize(const std::string& filePath);
	void Initialize(std::vector<Model2D>&& models);
	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	std::vector<Model2D> m_Models;
};

class Scene3D final
{
public:

	Scene3D() = default;
	~Scene3D() = default;

	void Initialize(const std::string& filePath);
	void Initialize(std::vector<Model3D>&& models);
	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	std::vector<Model3D> m_Models;

};

class Scene3DIR final
{
public:

	Scene3DIR() = default;
	~Scene3DIR() = default;

	// Add move constructor
	Scene3DIR(Scene3DIR&& other) noexcept
	{
		// move models
		m_Models = std::move(other.m_Models);

		// Invalidate the moved-from object
		other.m_Models.clear();
	}

	// Add move assignment operator
	Scene3DIR& operator=(Scene3DIR&& other) noexcept
	{
		if (this != &other)
		{
			// Move resources from other
			m_Models = std::move(other.m_Models);

			// Invalidate the moved-from object
			other.m_Models.clear();
		}
		return *this;
	}

	void Initialize(const VulkanInstance& instance, const CommandPool& commandPool, const std::string& filePath);
	void Initialize(std::vector<Model3DIR>&& models);
	void Destroy(VkDevice device);

	void Update(VkDevice device);
	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	std::vector<Model3DIR> m_Models;

};

#endif // !SCENE_H