#ifndef VULKANBASE_H
#define	VULKANBASE_H

#include <iostream>
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>

#include <vulkan/vulkan.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanStructs.h"
#include "VulkanUtils.h"

// Abstractions //
#include "RenderPass.h"
#include "CommandPool.h"
#include "DataBuffer.h"
#include "Image.h"
#include "ImageView.h"
#include "Sampler.h"
#include "Model.h"
#include "Shader.h"


#include "Camera.h"

class VulkanBase final
{
public:

	VulkanBase() = default;
	~VulkanBase() = default;

	void Run();

private:

	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();

	void DrawFrame();

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	// Instances And Debug Messeges
	void CreateInstance();
	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	std::vector<const char*> VulkanBase::GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);

	// Phicsical Devices
	void PickPhysicalDevice();
	void CreateLogicDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	// Surface
	void CreateSurface();

	// SwapChain
	void CreateSwapChain();
	void RecreateSwapChain();
	void CreateSwapChainImageViews();
	void CreateFramebuffers();
	void CleanupSwapChain();
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// RenderPass
	void CreateRenderPass();

	// Graphics Pipeline
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipelineLayout();
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	// Command Pool
	void CreateCommandPool();
	void CreateCommandBuffer();
	void RecordCommandBuffer(CommandBuffer commandBuffer, uint32_t imageIndex);
	
	//SyncObjects
	void CreateSyncObjects();
	void CleanupSyncObjects();

	// Creating buffers
	void CreateModel();

	// Camera
	void CreateCamera();
	void CleanupCamera();

	// DescriptorPool
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	// Textures
	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();

	// Depth Buffer
	void CreateDepthResources();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);

private:

	// Window
	GLFWwindow* m_Window;

	// Instances And Debug Messeges
	VkInstance m_VulkanInstance;
#ifdef NDEBUG
	const bool m_ValidationLayersEnabled{ false };
#else
	const bool m_ValidationLayersEnabled{ true };
#endif
	VkDebugUtilsMessengerEXT m_DebugMessenger;

	// Physical Device
	VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
	VkDevice m_Device;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	// Window Surface
	VkSurfaceKHR m_Surface;

	// SwapChain
	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages; // All SwapChain Images
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent; // SwapChain Image Resolution
	std::vector<ImageView> m_SwapChainImageViews;

	// RenderPass
	RenderPass m_RenderPass;

	// Pipeline
	VkPipeline m_GraphicsPipeline;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_PipelineLayout;

	// Frame Buffers
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	// CommandPool
	CommandPool m_CommandPool;
	std::vector<CommandBuffer> m_CommandBuffers;

	// Sync Objects
	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	bool m_FramebufferResized{ false };

	// Frames in flight
	uint32_t m_CurrentFrame{ 0 };

	// Model
	Model m_Model;

	// DescriptorPool
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	// Textures
	Image m_TextureImage;
	ImageView m_TextureImageView;
	Sampler m_TextureSampler;

	// Depth Buffer
	Image m_DepthImage;
	ImageView m_DepthImageView;

	// Camera
	Camera m_Camera;
};

#endif // !VULKANBASE_H