#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "vulkan/vulkan.h"

class CommandPool;

class CommandBuffer final
{
public:

	CommandBuffer();
	explicit CommandBuffer(VkCommandBuffer commandBuffer);
	~CommandBuffer() = default;

	void Destroy(VkDevice device, const CommandPool& commandPool);

	const VkCommandBuffer& GetVkCommandBuffer() const;

	void Reset(VkCommandBufferResetFlags flags = 0) const;
	void BeginRecording(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;
	void EndRecording() const;

	void BeginRenderPass(const VkRenderPassBeginInfo& renderPassInfo, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
	void EndRenderPass() const;

	void Submit(VkQueue queue, VkFence fence) const;
	void Submit(VkSubmitInfo& submitInfo, VkQueue queue, VkFence fence) const;

private:

	VkCommandBuffer m_VkCommandBuffer;

};


#endif // !COMMANDBUFFER_H