#ifndef DEPTHBUFFER_H
#define DEPTHBUFFER_H

#include <vector>

#include <vulkan/vulkan.h>

#include "Image.h"
#include "ImageView.h"

class CommandPool;

class DepthBuffer final
{
public:

	DepthBuffer() = default;
	~DepthBuffer() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue GraphQueue, const CommandPool& cmndP, uint32_t width, uint32_t height);
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