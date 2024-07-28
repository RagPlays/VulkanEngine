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
#include "GraphicsPipeline3DIR.h"
#include "DepthBuffer.h"
#include "Swapchain.h"

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

	void Update();
	void DrawFrame();
	void RecordCommandBuffer(uint32_t imageIndex);

	void CleanupWindowResources();
	void RecreateWindowResources();

	// Frame Buffers
	void CreateFramebuffers();

	// Graphics Pipeline
	void CreateGraphicsPipeline2D();
	void CreateGraphicsPipeline3D();
	void CreateGraphicsPipeline3DIR();

	// Command Pool
	void CreateCommandBuffers();

	// Scenes
	void Create2DScene();
	void Create3DScene();
	void Create3DIRScene();

private:

	// Window
	Window m_Window;

	// Instances(debug messages) & Devices
	VulkanInstance m_VulkanInstance;

	// SwapChain
	Swapchain m_Swapchain;

	// RenderPass
	RenderPass m_RenderPass;

	// Pipeline
	GraphicsPipeline2D m_GraphicsPipeline2D;
	GraphicsPipeline3D m_GraphicsPipeline3D;
	GraphicsPipeline3DIR m_GraphicsPipeline3DIR;

	// Frame Buffers
	std::vector<VkFramebuffer> m_FrameBuffers;

	// CommandPool
	CommandPool m_CommandPool;
	std::vector<CommandBuffer> m_CommandBuffers;

	// Sync Objects
	SyncObjects m_SyncObjects;

	// Frames in flight
	uint32_t m_CurrentFrame;

	// Textures
	Texture m_3DTexture;
	Texture m_3DIRTexture;

	// Depth Buffer
	DepthBuffer m_DepthBuffer;

	// Camera
	Camera m_Camera;
};

#endif // !VULKANBASE_H