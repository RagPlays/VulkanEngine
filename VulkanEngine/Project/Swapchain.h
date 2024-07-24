#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vector>

#include <vulkan/vulkan.h>

#include "ImageView.h"

class VulkanInstance;
class Window;

class Swapchain final
{
public:

	Swapchain() = default;
	~Swapchain() = default;

	void Initialize(const VulkanInstance& instance, const Window& window);
	void Destroy(VkDevice device);

	const VkSwapchainKHR& GetVkSwapchain() const;
	const VkFormat& GetVkFormat() const;
	const VkExtent2D& GetVkExtent() const;
	const std::vector<ImageView>& GetImageViews() const;

private:

	void CreateSwapchain(const VulkanInstance& instance, const Window& window);
	void CreateSwapchainImageViews(VkDevice device);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window) const;

private:

	VkSwapchainKHR m_VkSwapChain;
	std::vector<VkImage> m_VkImages; // All SwapChain Images // not Image Class because its raw pointers from swapchain
	std::vector<ImageView> m_ImageViews;
	VkFormat m_VkFormat;
	VkExtent2D m_VkExtent; // SwapChain Image Resolution

};

#endif // !SWAPCHAIN_H