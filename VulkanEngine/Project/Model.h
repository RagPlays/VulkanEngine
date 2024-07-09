#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "VulkanStructs.h"
#include "DataBuffer.h"
#include "Texture.h"

#include "Vertex.h"

class Camera;

class Model2D final
{
public:

	Model2D();
	~Model2D() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndP, const std::string& modelFilePath);
	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndP, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices);
	void Destroy(VkDevice device);

	void SetPosition(const glm::vec2& pos);
	void SetRotation(float angle);
	void SetScale(const glm::vec2& scale);
	void SetScale(float scale);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex2D>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices);

	void UpdateModelMatrix();

private:

	Transform2D m_Transform;
	Model2DUBO m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

class Model3D final
{
public:

	Model3D();
	~Model3D() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndP, const std::string& modelFilePath);
	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& cmndP, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);
	void Destroy(VkDevice device);

	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::quat& rotation);
	void SetScale(const glm::vec3& scale);
	void SetScale(float scale);
	void SetTranform(const Transform3D& transform);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(VkDevice device, VkPhysicalDevice phyDevice, VkQueue queue, const CommandPool& commandPool, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);

	void UpdateModelMatrix();

private:

	Transform3D m_Transform;
	Model3DUBO m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

#endif // !MODEL_H
