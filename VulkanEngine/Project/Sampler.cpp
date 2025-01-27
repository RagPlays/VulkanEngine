#include <stdexcept>

#include "Sampler.h"

Sampler::Sampler()
	: m_VkSampler{ VK_NULL_HANDLE }
{
}

void Sampler::Initialize(VkDevice device, VkPhysicalDevice phyDevice)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(phyDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	/*samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;*/

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &m_VkSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create sampler!");
	}
}

void Sampler::Destroy(VkDevice device)
{
	if (m_VkSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device, m_VkSampler, VK_NULL_HANDLE);
		m_VkSampler = VK_NULL_HANDLE;
	}
}

const VkSampler& Sampler::GetVkSampler() const
{
	return m_VkSampler;
}