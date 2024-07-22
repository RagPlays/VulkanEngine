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
#include "SyncObjects.h"
#include "VulkanInstance.h"
#include "Surface.h"
#include "Camera.h"
#include "Scene.h"
#include "Texture.h"
#include "Window.h"
#include "GraphicsPipeline2D.h"
#include "GraphicsPipeline3D.h"
#include "DepthBuffer.h"

class Application final
{
public:

	Application() = default;
	~Application() = default;

	void Run();

private:

	void InitVulkan();
	void MainLoop();
	void Cleanup();

	void DrawFrame();
	void RecordCommandBuffer(CommandBuffer commandBuffer, uint32_t imageIndex);

	// Phicsical Devices
	void PickPhysicalDevice();
	void CreateLogicDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

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

	// Graphics Pipeline
	void CreateGraphicsPipeline2D();
	void CreateGraphicsPipeline3D();

	// Command Pool
	void CreateCommandPool();
	void CreateCommandBuffers();

	// Scene
	void CreateScenes();

private:

	// Window
	Window m_Window;

	// Instances And Debug Messeges
	VulkanInstance m_VulkanInstance;

	// Surface
	Surface m_Surface;

	// Devices
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;

	// SwapChain
	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages; // All SwapChain Images
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent; // SwapChain Image Resolution
	std::vector<ImageView> m_SwapChainImageViews;

	// RenderPass
	RenderPass m_RenderPass;

	// Pipeline
	GraphicsPipeline2D m_GraphicsPipeline2D;
	GraphicsPipeline3D m_GraphicsPipeline3D;

	// Frame Buffers
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;

	// CommandPool
	CommandPool m_CommandPool;
	std::vector<CommandBuffer> m_CommandBuffers;

	// Sync Objects
	SyncObjects m_SyncObjects;

	// Frames in flight
	uint32_t m_CurrentFrame;

	// Textures
	Texture m_Texture;

	// Depth Buffer
	DepthBuffer m_DepthBuffer;

	// Camera
	Camera m_Camera;
};

#endif // !VULKANBASE_H