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

	Model();
	~Model() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::string& filePath);
	void Destroy(VkDevice device);

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::quat rotation);
	void SetRotation(glm::vec3 rotation);
	void SetScale(glm::vec3 scale);
	void SetScale(float scale);
	void SetTranform(const Transform& transform);

	const glm::mat4& GetModelMatrix() const;
	const std::vector<DataBuffer>& GetBuffers() const;

	void UpdateUniformBuffers(uint32_t currentFrame);
	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	void InitUBOBuffers(VkDevice device, VkPhysicalDevice phyDevice);

	void UpdateModelMatrix();

private:

	std::vector<DataBuffer> m_ModelUBOs;
	std::vector<void*> m_ModelUBOsMapped;

	Transform m_Transform;
	glm::mat4 m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

#endif // !MODEL_H
