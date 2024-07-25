#ifndef DEPTHBUFFER_H
#define DEPTHBUFFER_H

#include <vector>

#include <vulkan/vulkan.h>

#include "Image.h"
#include "ImageView.h"

class CommandPool;
class VulkanInstance;
class Swapchain;

class DepthBuffer final
{
public:

	DepthBuffer() = default;
	~DepthBuffer() = default;

	void Initialize(const VulkanInstance& instance, const CommandPool& commandPool, const Swapchain& swapchain);
	void Destroy(VkDevice device);

	const VkImageView& GetVkImageView() const;
	const VkFormat& GetDepthFormat() const;

private:

	VkFormat FindSupportedFormat(VkPhysicalDevice phyDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat(VkPhysicalDevice phyDevice);
	bool HasStencilComponent(VkFormat format) const;


	VkFormat m_DepthFormat;
	Image m_DepthImage;
	ImageView m_DepthImageView;

};

#endif // !DEPTHBUFFER_H