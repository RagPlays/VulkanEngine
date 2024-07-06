#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "VulkanStructs.h"
#include "DataBuffer.h"

class Camera;

class Model final
{
public:

	Model() = default;
	~Model() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::string& filePath);
	void Destroy(VkDevice device);

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::quat rotation);
	void SetScale(glm::vec3 scale);
	void SetTranform(const Transform& transform);

	void Draw(VkCommandBuffer commandBuffer, Camera* camera) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	void UpdateModelMatrix();

private:

	Transform m_Transform;
	glm::mat4 m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

#endif // !MODEL_H
