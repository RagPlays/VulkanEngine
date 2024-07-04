#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H

#include <vulkan/vulkan_core.h>
#include "CommandBuffer.h"

struct QueueFamilyIndices;

class CommandPool
{
public:

	CommandPool();
	~CommandPool() = default;

	void Initialize(VkDevice device, const QueueFamilyIndices& queue);
	void Destroy(VkDevice device);

	CommandBuffer CreateCommandBuffer(VkDevice device, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
	const VkCommandPool& GetVkCommandPool() const;

private:

	VkCommandPool m_VkCommandPool;

};

#endif // !COMMANDPOOL_H