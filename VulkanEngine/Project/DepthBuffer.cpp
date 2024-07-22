#include <stdexcept>

#include "DepthBuffer.h"

void DepthBuffer::Initialize(VkDevice device, VkPhysicalDevice phyDevice, VkQueue GraphQueue, const CommandPool& cmndP, uint32_t width, uint32_t height)
{
	m_DepthFormat = FindDepthFormat(phyDevice);

	m_DepthImage.Initialize(device, phyDevice, width, height, m_DepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	m_DepthImageView.Initialize(device, m_DepthImage.GetVkImage(), m_DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	m_DepthImage.TransitionImageLayout(device, cmndP, GraphQueue, m_DepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void DepthBuffer::Destroy(VkDevice device)
{
	m_DepthImage.Destroy(device);
	m_DepthImageView.Destroy(device);
}

const VkImageView& DepthBuffer::GetVkImageView() const
{
	return m_DepthImageView.GetVkImageView();
}

const VkFormat& DepthBuffer::GetDepthFormat() const
{
	return m_DepthFormat;
}

VkFormat DepthBuffer::FindSupportedFormat(VkPhysicalDevice phyDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (const VkFormat& format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(phyDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat DepthBuffer::FindDepthFormat(VkPhysicalDevice phyDevice)
{
	return FindSupportedFormat(phyDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool DepthBuffer::HasStencilComponent(VkFormat format) const
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}