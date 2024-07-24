#ifndef VULKANINSTANCE_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "Surface.h"
#include "VulkanStructs.h"

using MessageCreateInfo = VkDebugUtilsMessengerCreateInfoEXT;
using MessageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT;
using MessageType = VkDebugUtilsMessageTypeFlagsEXT;
using MessageData = VkDebugUtilsMessengerCallbackDataEXT;
using UtilsMessengerCreateInfo = VkDebugUtilsMessengerCreateInfoEXT;
using UtilsMessenger = VkDebugUtilsMessengerEXT;
using AllocCallbacks = VkAllocationCallbacks;

class VulkanInstance final
{
public:

	VulkanInstance() = default;
	~VulkanInstance() = default;

	void Initialize(GLFWwindow* window);
	void Destroy();


	// Instance
	const VkInstance& GetVkInstance() const;
	bool GetValidationLayersEnabled() const;

	// Surface
	const VkSurfaceKHR& GetVkSurface() const;

	// Devices
	const VkDevice& GetVkDevice() const;
	const VkPhysicalDevice& GetVkPhysicalDevice() const;
	const VkQueue& GetGraphicsQueue() const;
	const VkQueue& GetPresentQueue() const;
	VkResult DeviceWaitIdle();

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;

private:

	void CreateVulkanInstance();
	void CreateSurface(GLFWwindow* window);
	void CreateDevices();

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(MessageCreateInfo& createInfo);
	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(MessageSeverity messgSeverity, MessageType messgType, const MessageData* callbakcD, void* pUserData);

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const UtilsMessengerCreateInfo* pCreateInfo, const AllocCallbacks* pAllocator, UtilsMessenger* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, UtilsMessenger debugMessenger, const AllocCallbacks* pAllocator);

	// Devices
	void PickPhysicalDevice();
	void CreateLogicDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);

private:

	VkInstance m_VulkanInstance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;

#ifdef NDEBUG
	static const bool m_ValidationLayersEnabled{ false };
#else
	static const bool m_ValidationLayersEnabled{ true };
#endif

	static const std::vector<const char*> s_DeviceExtensions;
	static const std::vector<const char*> s_ValidationLayers;

	static const std::string s_AppName;
	static const std::string s_EngineName;

	// Surface
	Surface m_Surface;

	// Devices
	VkPhysicalDevice m_VkPhysicalDevice;
	VkDevice m_VkDevice;
	VkQueue m_GraphicsVkQueue;
	VkQueue m_PresentVkQueue;
};

#endif // !VULKANINSTANCE_H