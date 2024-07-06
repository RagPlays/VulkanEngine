#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Model.h"

class Camera;

class Scene final
{
public:

	Scene() = default;
	~Scene() = default;

	void Initialize(const std::string& filePath);
	void Initialize(std::vector<Model>&& models);
	void Destroy(VkDevice device);

	void UpdateBuffers(uint32_t currentFrame);
	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);

	size_t GetNrOfModels() const;
	const std::vector<Model>& GetModels() const;

private:

	std::vector<Model> m_Models;

};

#endif // !SCENE_H