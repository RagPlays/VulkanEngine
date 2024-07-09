#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Model.h"

class Camera;

class Scene2D final
{
public:

	Scene2D() = default;
	~Scene2D() = default;

	void Initialize(const std::string& filePath);
	void Initialize(std::vector<Model2D>&& models);
	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

	size_t GetNrOfModels() const;
	const std::vector<Model2D>& GetModels() const;

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

	size_t GetNrOfModels() const;
	const std::vector<Model3D>& GetModels() const;

private:

	std::vector<Model3D> m_Models;

};

#endif // !SCENE_H