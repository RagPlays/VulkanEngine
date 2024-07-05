#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Model.h"

class Scene final
{
public:

	Scene() = default;
	~Scene() = default;

	void Initialize(const std::string& filePath);
	void Initialize(std::vector<Model>&& models);

	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer);

private:

private:

	std::vector<Model> m_Models;

};

#endif // !SCENE_H