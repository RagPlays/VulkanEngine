#ifndef IMAGE_H
#define IMAGE_H

#include <vulkan/vulkan.h>

class CommandPool;
class DataBuffer;

class Image final
{
public:

	Image();
	~Image() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags prop);
	void Destroy(VkDevice device);

	const VkImage& GetVkImage() const;
	const VkDeviceMemory& GetVkDeviceMemory() const;
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;

	void TransitionImageLayout(VkDevice device, const CommandPool& commandPool, VkQueue queue, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkDevice device, const DataBuffer& buffer, const CommandPool& commandPool, VkQueue queue);

	static bool HasStencilComponent(VkFormat format);

private:

	static uint32_t FindMemoryType(VkPhysicalDevice physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	
private:

	uint32_t m_Width;
	uint32_t m_Heigth;
	VkImage m_VkImage;
	VkDeviceMemory m_VkImageMemory;

};

#endif // !IMAGE_H
