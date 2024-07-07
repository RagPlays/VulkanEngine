#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "VulkanStructs.h"
#include "DataBuffer.h"

class Camera;

class Model3D final
{
public:

	Model3D();
	~Model3D() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndP, const std::string& modelFilePath);
	void Destroy(VkDevice device);

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::quat rotation);
	void SetScale(glm::vec3 scale);
	void SetScale(float scale);
	void SetTranform(const Transform3D& transform);

	const std::vector<DataBuffer>& GetBuffers() const;

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);

	void InitUBOBuffers(VkDevice device, VkPhysicalDevice phyDevice);

	void UpdateModelMatrix();

private:

	std::vector<DataBuffer> m_ModelUBOs;
	std::vector<void*> m_ModelUBOsMapped;

	Transform3D m_Transform;
	ModelUBO m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

#endif // !MODEL_H
