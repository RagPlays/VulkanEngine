#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <vulkan/vulkan.h>

class CommandPool;

class DataBuffer final
{
public:

	DataBuffer();
	~DataBuffer() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBufferUsageFlags usage);
	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkMemoryPropertyFlags properties, const VkBufferCreateInfo& bufferCreateInfo);
	void Destroy(VkDevice device);

	const VkBuffer& GetVkBuffer() const;
	const VkDeviceMemory& GetVkDeviceMemory() const;
	const VkDeviceSize& GetSizeInBytes() const;

	void Upload(VkDevice device, VkDeviceSize size, const void* data) const;
	void Map(VkDevice device, VkDeviceSize size, void** data) const;
	void BindAsVertexBuffer(VkCommandBuffer commandBuffer, uint32_t firstBinding = 0) const;
	void BindAsIndexBuffer(VkCommandBuffer commandBuffer) const;

	static void CopyBuffer(VkQueue graphicsQueue, VkDevice device, const CommandPool& commandPool, DataBuffer srcBuffer, DataBuffer dstBuffer, VkDeviceSize size);

private:

	uint32_t FindMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

private:

	VkDeviceSize m_Size;
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkBufferMemory;

};

#endif // !DATABUFFER_H