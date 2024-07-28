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
class VulkanInstance;

class Model2D final
{
public:

	Model2D();
	~Model2D() = default;

	void Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::string& modelFilePath);
	void Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices);
	void Destroy(VkDevice device);

	void SetPosition(const glm::vec2& pos);
	void SetRotation(float angle);
	void SetScale(const glm::vec2& scale);
	void SetScale(float scale);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex2D>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(const VulkanInstance& instance, const CommandPool& commandPool, const std::vector<Vertex2D>& vertices, const std::vector<uint32_t>& indices);

	void UpdateModelMatrix();

private:

	Transform2D m_Transform;
	ModelUBO m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

class Model3D final
{
public:

	Model3D();
	~Model3D() = default;

	void Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::string& modelFilePath);
	void Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);
	void Destroy(VkDevice device);

	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::quat& rotation);
	void SetScale(const glm::vec3& scale);
	void SetScale(float scale);
	void SetTranform(const Transform3D& transform);

	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex3D>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(const VulkanInstance& instance, const CommandPool& commandPool, const std::vector<Vertex3D>& vertices, const std::vector<uint32_t>& indices);

	void UpdateModelMatrix();

private:

	Transform3D m_Transform;
	ModelUBO m_ModelMatrix;
	uint32_t m_NrIndices;
	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;

};

class Model3DIR final
{
public:

	Model3DIR();
	~Model3DIR() = default;

	void Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::string& modelFilePath, uint32_t instanceCount);
	void Initialize(const VulkanInstance& instance, const CommandPool& cmndP, const std::vector<Vertex3DIR>& vertices, const std::vector<uint32_t>& indices, uint32_t instanceCount);
	void Destroy(VkDevice device);

	void SetPosition(const glm::vec3& position);
	void SetPosition(uint32_t instanceIndex, const glm::vec3& position);

	void SetRotation(const glm::vec3& rotation);
	void SetRotation(uint32_t instanceIndex, const glm::vec3& rotation);

	void SetScale(const glm::vec3& scale);
	void SetScale(float scale);
	void SetScale(uint32_t instanceIndex, const glm::vec3& scale);
	void SetScale(uint32_t instanceIndex, float scale);

	void SetTransform(uint32_t instanceIndex, const Transform3D& transform);

	uint32_t GetInstanceCount() const;

	void Update(VkDevice device);
	void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

private:

	void LoadModelFromFile(const std::string& filePath, std::vector<Vertex3DIR>& vertices, std::vector<uint32_t>& indices);
	void InitDataBuffers(const VulkanInstance& instance, const CommandPool& commandPool, const std::vector<Vertex3DIR>& vertices, const std::vector<uint32_t>& indices);
	void UpdateModelMatrix(uint32_t instanceIndex);
	void UpdateModelBuffer(VkDevice device) const;

private:

	std::vector<Transform3D> m_Transforms;
	std::vector<ModelUBO> m_ModelMatrices;

	uint32_t m_InstanceCount;
	uint32_t m_NrIndices;

	DataBuffer m_VertexBuffer;
	DataBuffer m_IndexBuffer;
	DataBuffer m_InstanceBuffer;

};
#endif // !MODEL_H
