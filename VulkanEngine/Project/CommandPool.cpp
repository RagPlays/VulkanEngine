#include <stdexcept>

#include "CommandPool.h"

#include "VulkanStructs.h"

CommandPool::CommandPool()
	: m_VkCommandPool{ VK_NULL_HANDLE }
{
}

void CommandPool::Initialize(VkDevice device, const QueueFamilyIndices& queue)
{
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT (Hint that command buffers are rerecorded with new commands very often)
	// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT (Allow command buffers to be rerecorded individually)
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queue.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void CommandPool::Destroy(VkDevice device)
{
	vkDestroyCommandPool(device, m_VkCommandPool, nullptr);
}

CommandBuffer CommandPool::CreateCommandBuffer(VkDevice device, VkCommandBufferLevel level) const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_VkCommandPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer vkCommandBuffer{ VK_NULL_HANDLE };
	if (vkAllocateCommandBuffers(device, &allocInfo, &vkCommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return CommandBuffer{ vkCommandBuffer };
}

const VkCommandPool& CommandPool::GetVkCommandPool() const
{
	return m_VkCommandPool;
}