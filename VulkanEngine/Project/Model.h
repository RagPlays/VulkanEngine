#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "VulkanStructs.h"
#include "DataBuffer.h"

class Model final
{
public:

	Model() = default;
	~Model() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::string& filePath);
	void Destroy(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer) const;

private:

	void InitVerticesIndices(const std::string& filePath);
	void InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool);

private:

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

#endif // !MODEL_H
