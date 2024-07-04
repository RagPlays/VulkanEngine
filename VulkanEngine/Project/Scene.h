#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "Model.h"

class Scene final
{
public:

	Scene() = default;
	~Scene() = default;

	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer commandBuffer, VkDescriptorSet discriptorSet);

private:

private:

	std::vector<Model> m_Models;

};

#endif // !SCENE_H