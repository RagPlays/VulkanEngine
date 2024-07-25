#include <stdexcept>
#include <algorithm>

#include "Swapchain.h"
#include "VulkanInstance.h"
#include "Window.h"

void Swapchain::Initialize(const VulkanInstance& instance, const Window& window)
{
	CreateSwapchain(instance, window);
	CreateSwapchainImageViews(instance.GetVkDevice());
}

void Swapchain::Destroy(VkDevice device)
{
	for (auto imageview : m_ImageViews)
	{
		imageview.Destroy(device);
	}
	m_ImageViews.clear();

	if (m_VkSwapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, m_VkSwapChain, VK_NULL_HANDLE);
		m_VkSwapChain = VK_NULL_HANDLE;
	}
}

const VkSwapchainKHR& Swapchain::GetVkSwapchain() const
{
	return m_VkSwapChain;
}

const VkFormat& Swapchain::GetVkFormat() const
{
	return m_VkFormat;
}

const VkExtent2D& Swapchain::GetVkExtent() const
{
	return m_VkExtent;
}

const std::vector<ImageView>& Swapchain::GetImageViews() const
{
	return m_ImageViews;
}

void Swapchain::CreateSwapchain(const VulkanInstance& instance, const Window& window)
{
	const VkDevice& device{ instance.GetVkDevice() };
	const VkSurfaceKHR& surface{ instance.GetVkSurface() };

	SwapChainSupportDetails swapChainSupport{ instance.QuerySwapChainSupport() };

	VkSurfaceFormatKHR surfaceFormat{ ChooseSwapSurfaceFormat(swapChainSupport.formats) };
	VkPresentModeKHR presentMode{ ChooseSwapPresentMode(swapChainSupport.presentModes) };
	VkExtent2D extent{ ChooseSwapExtent(swapChainSupport.capabilities, window) };

	uint32_t imageCount{ swapChainSupport.capabilities.minImageCount + 1 }; // recommended minimum + 1 (otherwise bottleneck)
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) // check to not exceed max
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	QueueFamilyIndices indices{ instance.FindQueueFamilies() };
	uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // Images can be used across multiple queue families (no ownership)
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // An image is owned by one queue family (ownership can be moved)
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ignoring alpha channel
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // Enabling clipping (best performance)

	createInfo.oldSwapchain = VK_NULL_HANDLE; // Needed when recreating swapchain (window resizing, etc..)

	if (vkCreateSwapchainKHR(device, &createInfo, VK_NULL_HANDLE, &m_VkSwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create swap chain!" };
	}

	if (vkGetSwapchainImagesKHR(device, m_VkSwapChain, &imageCount, VK_NULL_HANDLE) != VK_SUCCESS) // get only size
	{
		throw std::runtime_error{ "failed to get swap chain images count!" };
	}
	m_VkImages.resize(imageCount);
	if(vkGetSwapchainImagesKHR(device, m_VkSwapChain, &imageCount, m_VkImages.data()) != VK_SUCCESS) // get swap chain image data
	{
		throw std::runtime_error{ "failed to get swap chain images data!" };
	}

	// Store chosen format and extent
	m_VkFormat = surfaceFormat.format; // VK_FORMAT_B8G8R8A8_SRGB
	m_VkExtent = extent; // chosen resution of swap chain (width, height)
}

void Swapchain::CreateSwapchainImageViews(VkDevice device)
{
	m_ImageViews.resize(m_VkImages.size());
	for (size_t idx{}; idx < m_VkImages.size(); ++idx)
	{
		ImageView imageView{};
		imageView.Initialize(device, m_VkImages[idx], m_VkFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		m_ImageViews[idx] = imageView;
	}
}

VkSurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR Swapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
{
	// VK_PRESENT_MODE_IMMEDIATE_KHR // Can have screen tearing
	// VK_PRESENT_MODE_FIFO_KHR // More like VSync
	// VK_PRESENT_MODE_FIFO_RELAXED_KHR // Force inserts it, can have teering
	// VK_PRESENT_MODE_MAILBOX_KHR // Queue get replaced (triple buffering)

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window) const
{
	// Swap extent is the resolution of the swap chain images //

	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width{};
		int height{};
		window.GetFramebufferSize(width, height);

		VkExtent2D actualExtent
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
