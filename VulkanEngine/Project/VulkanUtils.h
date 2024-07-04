#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#include <vector>
#include <string>
#include <fstream>

#include <vulkan/vulkan.h>

const int MAX_FRAMES_IN_FLIGHT{ 2 };

// 720p resolution (AR 16:9)
constexpr uint32_t g_WindowWidth{ 1280 };
constexpr uint32_t g_WindowHeight{ 720 };
constexpr float g_AspectRatio{ g_WindowWidth / static_cast<float>(g_WindowHeight) };

const std::string g_ModelPath{ "Resources/Models/viking_room.obj" };
const std::string g_TexturePath{ "Resources/Models/viking_room.png" };

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