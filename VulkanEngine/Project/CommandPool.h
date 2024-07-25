#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H

#include <vulkan/vulkan_core.h>
#include "CommandBuffer.h"

class VulkanInstance;

class CommandPool
{
public:

	CommandPool();
	~CommandPool() = default;

	void Initialize(const VulkanInstance& instance);
	void Destroy(VkDevice device);

	CommandBuffer CreateCommandBuffer(VkDevice device, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
	const VkCommandPool& GetVkCommandPool() const;

private:

	VkCommandPool m_VkCommandPool;

};

#endif // !COMMANDPOOL_H