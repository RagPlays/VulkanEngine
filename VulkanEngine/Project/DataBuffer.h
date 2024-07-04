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

	void Upload(VkDevice device, VkDeviceSize size, const void* data);
	void Map(VkDevice device, VkDeviceSize size, void** data);
	void BindAsVertexBuffer(VkCommandBuffer commandBuffer);
	void BindAsIndexBuffer(VkCommandBuffer commandBuffer);

	static void CopyBuffer(VkQueue graphicsQueue, VkDevice device, const CommandPool& commandPool, DataBuffer srcBuffer, DataBuffer dstBuffer, VkDeviceSize size);

private:

	static uint32_t FindMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:

	VkDeviceSize m_Size;
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkBufferMemory;

};

#endif // !DATABUFFER_H