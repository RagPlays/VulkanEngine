#ifndef VULKANINSTANCE_H

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

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

	bool GetValidationLayersEnabled() const;

	static uint32_t GetExtensionCount();
	static const char* const* GetExtionsionNames();

	static uint32_t GetValidationLayersCount();
	static const char* const* GetValidationLayersNames();

	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	const VkInstance& GetVkInstance() const;

private:

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(MessageCreateInfo& createInfo);
	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(MessageSeverity messgSeverity, MessageType messgType, const MessageData* callbakcD, void* pUserData);

	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const UtilsMessengerCreateInfo* pCreateInfo, const AllocCallbacks* pAllocator, UtilsMessenger* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, UtilsMessenger debugMessenger, const AllocCallbacks* pAllocator);

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
};

#endif // !VULKANINSTANCE_H