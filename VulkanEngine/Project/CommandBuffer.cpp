#include <stdexcept>

#include "CommandBuffer.h"
#include "CommandPool.h"

CommandBuffer::CommandBuffer()
    : m_VkCommandBuffer{ VK_NULL_HANDLE }
{
}

CommandBuffer::CommandBuffer(VkCommandBuffer commandBuffer)
    : m_VkCommandBuffer{ commandBuffer }
{
}

void CommandBuffer::Destroy(VkDevice device, const CommandPool& commandPool)
{
    vkFreeCommandBuffers(device, commandPool.GetVkCommandPool(), 1, &m_VkCommandBuffer);
}

const VkCommandBuffer& CommandBuffer::GetVkCommandBuffer() const
{
    return m_VkCommandBuffer;
}

void CommandBuffer::Reset(VkCommandBufferResetFlags flags) const
{
    vkResetCommandBuffer(m_VkCommandBuffer, flags);
}

void CommandBuffer::BeginRecording(VkCommandBufferUsageFlags usage) const
{
    // optional flags //
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT (The command buffer will be rerecorded right after executing it once)
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT (This is a secondary command buffer that will be entirely within a single render pass)
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT (The command buffer can be resubmitted while it is also already pending execution)

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage;

    if (vkBeginCommandBuffer(m_VkCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void CommandBuffer::EndRecording() const
{
    if (vkEndCommandBuffer(m_VkCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo& renderPassInfo, VkSubpassContents contents)
{
    vkCmdBeginRenderPass(m_VkCommandBuffer, &renderPassInfo, contents);
}

void CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(m_VkCommandBuffer);
}

void CommandBuffer::Submit(VkQueue queue, VkFence fence) const
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    Submit(submitInfo, queue, fence);
}

void CommandBuffer::Submit(VkSubmitInfo& submitInfo, VkQueue queue, VkFence fence) const
{
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_VkCommandBuffer;

    if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer!");
    }
    vkQueueWaitIdle(queue);
}