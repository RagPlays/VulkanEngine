#include <set>
#include <stdexcept>
#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "VulkanInstance.h"

#include "VulkanUtils.h"

const std::vector<const char*> VulkanInstance::s_DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
const std::vector<const char*> VulkanInstance::s_ValidationLayers{ "VK_LAYER_KHRONOS_validation" };

const std::string VulkanInstance::s_AppName{ "VulkanApplication" };
const std::string VulkanInstance::s_EngineName{ "MorrogEngine" };

void VulkanInstance::Initialize(GLFWwindow* window)
{
	if (m_ValidationLayersEnabled && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = s_AppName.c_str(); //"VulkanEngine"
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = s_EngineName.c_str(); //"MorrogEngine"
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions{ GetRequiredExtensions() };
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (m_ValidationLayersEnabled)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = s_ValidationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanInstance::Destroy()
{
	if (m_ValidationLayersEnabled) DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
	vkDestroyInstance(m_VulkanInstance, nullptr);
}

bool VulkanInstance::GetValidationLayersEnabled() const
{
	return m_ValidationLayersEnabled;
}

uint32_t VulkanInstance::GetExtensionCount()
{
	return static_cast<uint32_t>(s_DeviceExtensions.size());
}

const char* const* VulkanInstance::GetExtionsionNames()
{
	return s_DeviceExtensions.data();
}

uint32_t VulkanInstance::GetValidationLayersCount()
{
	return static_cast<uint32_t>(s_ValidationLayers.size());
}

const char* const* VulkanInstance::GetValidationLayersNames()
{
	return s_ValidationLayers.data();
}

const VkInstance& VulkanInstance::GetVkInstance() const
{
	return m_VulkanInstance;
}

void VulkanInstance::SetupDebugMessenger()
{
	if (!m_ValidationLayersEnabled) return;

	MessageCreateInfo createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(m_VulkanInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void VulkanInstance::PopulateDebugMessengerCreateInfo(MessageCreateInfo& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr; // Optional
}

std::vector<const char*> VulkanInstance::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount{};
	const char** glfwExtensions{ glfwGetRequiredInstanceExtensions(&glfwExtensionCount) };

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (m_ValidationLayersEnabled) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

bool VulkanInstance::CheckValidationLayerSupport()
{
	uint32_t layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers{ layerCount };
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : s_ValidationLayers)
	{
		bool layerFound{ false };

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}
	return true;
}

bool VulkanInstance::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount{};
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions{ extensionCount };
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions{ s_DeviceExtensions.begin(), s_DeviceExtensions.end() };

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::DebugCallback(MessageSeverity messgSeverity, MessageType messgType, const MessageData* callbakcD, void* pUserData)
{
	std::cerr << "validation layer: " << callbakcD->pMessage << "\n";
	return VK_FALSE;
}

VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const UtilsMessengerCreateInfo* pCreateInfo, const AllocCallbacks* pAllocator, UtilsMessenger* pDebugMessenger)
{
	auto func{ (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT") };
	if (func) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, UtilsMessenger debugMessenger, const AllocCallbacks* pAllocator)
{
	auto func{ (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT") };
	if (func) func(instance, debugMessenger, pAllocator);
}