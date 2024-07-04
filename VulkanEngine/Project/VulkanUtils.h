#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>

const int MAX_FRAMES_IN_FLIGHT{ 2 };

// 720p resolution (AR 16:9)
const uint32_t g_WindowWidth{ 1280 };
const uint32_t g_WindowHeight{ 720 };

const std::vector<const char*> g_ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> g_DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

VkResult CreateDebugUtilsMessengerEXT
(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
);

void DestroyDebugUtilsMessengerEXT
(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
);

std::vector<char> ReadFile(const std::string& filename);

#endif //!VULKANUTILS_H