#ifndef SAMPLER_H
#define SAMPLER_H

#include <vulkan/vulkan.h>

class Sampler final
{
public:

	Sampler();
	~Sampler() = default;

	void Initialize(VkDevice device, VkPhysicalDevice phyDevice);
	void Destroy(VkDevice device);

	const VkSampler& GetVkSampler() const;

private:

	VkSampler m_VkSampler;

};

#endif // !SAMPLER_H