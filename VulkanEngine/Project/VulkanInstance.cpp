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
	CreateVulkanInstance();
	CreateSurface(window);
	CreateDevices();
}

void VulkanInstance::Destroy()
{
	// Devices
	if (m_VkDevice != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_VkDevice, VK_NULL_HANDLE);
		m_VkDevice = VK_NULL_HANDLE;
	}

	if (m_VulkanInstance != VK_NULL_HANDLE)
	{
		// Surface
		m_Surface.Destroy(m_VulkanInstance);

		if (m_DebugMessenger != VK_NULL_HANDLE && m_ValidationLayersEnabled)
		{
			DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, VK_NULL_HANDLE);
		}
		m_DebugMessenger = VK_NULL_HANDLE;

		// Instance
		vkDestroyInstance(m_VulkanInstance, VK_NULL_HANDLE);
		m_VulkanInstance = VK_NULL_HANDLE;
	}
}

const VkInstance& VulkanInstance::GetVkInstance() const
{
	return m_VulkanInstance;
}

bool VulkanInstance::GetValidationLayersEnabled() const
{
	return m_ValidationLayersEnabled;
}

const VkSurfaceKHR& VulkanInstance::GetVkSurface() const
{
	return m_Surface.GetVkSurface();
}

const VkDevice& VulkanInstance::GetVkDevice() const
{
	return m_VkDevice;
}

const VkPhysicalDevice& VulkanInstance::GetVkPhysicalDevice() const
{
	return m_VkPhysicalDevice;
}

const VkQueue& VulkanInstance::GetGraphicsQueue() const
{
	return m_GraphicsVkQueue;
}

const VkQueue& VulkanInstance::GetPresentQueue() const
{
	return m_PresentVkQueue;
}

VkResult VulkanInstance::DeviceWaitIdle()
{
	return vkDeviceWaitIdle(m_VkDevice);
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

SwapChainSupportDetails VulkanInstance::QuerySwapChainSupport(VkPhysicalDevice phyDevice) const
{
	if (phyDevice == VK_NULL_HANDLE)
	{
		throw std::exception("undifined physical device");
	}

	const VkSurfaceKHR& surface{ m_Surface.GetVkSurface() };

	SwapChainSupportDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, surface, &details.capabilities);

	uint32_t formatCount{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &formatCount, nullptr);

	if (formatCount)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

SwapChainSupportDetails VulkanInstance::QuerySwapChainSupport() const
{
	return QuerySwapChainSupport(m_VkPhysicalDevice);
}

QueueFamilyIndices VulkanInstance::FindQueueFamilies(VkPhysicalDevice phyDevice) const
{
	if (phyDevice == VK_NULL_HANDLE)
	{
		throw std::exception("undifined physical device");
	}

	QueueFamilyIndices indices{};

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount, queueFamilies.data());

	const VkSurfaceKHR& surface{ m_Surface.GetVkSurface() };

	int idx{ 0 };
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = idx;
		}

		VkBool32 presentSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(phyDevice, idx, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = idx;
		}

		if (indices.IsComplete()) break;

		idx++;
	}

	return indices;
}

QueueFamilyIndices VulkanInstance::FindQueueFamilies() const
{
	return FindQueueFamilies(m_VkPhysicalDevice);
}

void VulkanInstance::CreateVulkanInstance()
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

void VulkanInstance::CreateSurface(GLFWwindow* window)
{
	m_Surface.Initialize(m_VulkanInstance, window);
}

void VulkanInstance::CreateDevices()
{
	PickPhysicalDevice();
	CreateLogicDevice();
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

void VulkanInstance::PickPhysicalDevice()
{
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices{ deviceCount };
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devices.data());

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			m_VkPhysicalDevice = device;
			break;
		}
	}
	if (m_VkPhysicalDevice == VK_NULL_HANDLE) throw std::runtime_error("failed to find a suitable GPU!");
}

void VulkanInstance::CreateLogicDevice()
{
	QueueFamilyIndices indices{ FindQueueFamilies(m_VkPhysicalDevice) };

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority{ 1.f };
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

	if (m_ValidationLayersEnabled)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = s_ValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_VkPhysicalDevice, &createInfo, nullptr, &m_VkDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_VkDevice, indices.graphicsFamily.value(), 0, &m_GraphicsVkQueue);
	vkGetDeviceQueue(m_VkDevice, indices.presentFamily.value(), 0, &m_PresentVkQueue);
}

bool VulkanInstance::IsDeviceSuitable(VkPhysicalDevice phyDevice)
{
	QueueFamilyIndices indices{ FindQueueFamilies(phyDevice) };
	bool extensionsSupported{ CheckDeviceExtensionSupport(phyDevice) };
	bool swapChainAdequate{ false };
	if (extensionsSupported)
	{
		const SwapChainSupportDetails swapChainSupport{ QuerySwapChainSupport(phyDevice) };
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures{};
	vkGetPhysicalDeviceFeatures(phyDevice, &supportedFeatures);

	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}