#include "SyncObjects.h"

#include "VulkanUtils.h"

void SyncObjects::Initialize(VkDevice device)
{
	m_ImageAvailableSemaphores.resize(g_MaxFramesInFlight);
	m_RenderFinishedSemaphores.resize(g_MaxFramesInFlight);
	m_InFlightFences.resize(g_MaxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int idx{}; idx < g_MaxFramesInFlight; idx++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[idx]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[idx]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[idx]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void SyncObjects::Destroy(VkDevice device)
{
	for (auto imageAbailableSemaphore : m_ImageAvailableSemaphores)
	{
		vkDestroySemaphore(device, imageAbailableSemaphore, nullptr);
	}
	for (auto renderFinishedSemaphore : m_RenderFinishedSemaphores)
	{
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	}
	for (auto inFlightFence : m_InFlightFences)
	{
		vkDestroyFence(device, inFlightFence, nullptr);
	}
}

const VkSemaphore& SyncObjects::GetImageAvailableSemaphore(uint32_t currentFrame)
{
	return m_ImageAvailableSemaphores[currentFrame];
}

const VkSemaphore& SyncObjects::GetRenderFinishedSemaphore(uint32_t currentFrame)
{
	return m_RenderFinishedSemaphores[currentFrame];
}

const VkFence& SyncObjects::GetInFlightFence(uint32_t currentFrame)
{
	return m_InFlightFences[currentFrame];
}