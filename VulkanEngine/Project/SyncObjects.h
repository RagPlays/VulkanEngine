#ifndef SYNCOBJECTS_H
#define SYNCOBJECTS_H

#include <vector>

#include <vulkan/vulkan.h>

class SyncObjects final
{
public:

	SyncObjects() = default;
	~SyncObjects() = default;

	void Initialize(VkDevice device);
	void Destroy(VkDevice device);

	const VkSemaphore& GetImageAvailableSemaphore(uint32_t currentFrame);
	const VkSemaphore& GetRenderFinishedSemaphore(uint32_t currentFrame);
	const VkFence& GetInFlightFence(uint32_t currentFrame);

private:

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;

};

#endif // !SYNCOBJECTS_H