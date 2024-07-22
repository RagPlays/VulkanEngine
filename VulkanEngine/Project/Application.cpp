#include "Application.h"
#include "Timer.h"

void Application::Run()
{
	InitVulkan();
	MainLoop();
	Cleanup();
}

void Application::InitVulkan()
{
	m_Window.Initialize();

	m_VulkanInstance.Initialize(m_Window.GetWindow());
	m_Surface.Initialize(m_VulkanInstance.GetVkInstance(), m_Window.GetWindow());

	PickPhysicalDevice();
	CreateLogicDevice();

	CreateSwapChain();
	CreateSwapChainImageViews();

	CreateCommandPool();
	CreateCommandBuffers();

	m_DepthBuffer.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, m_SwapChainExtent.width, m_SwapChainExtent.height);

	m_RenderPass.Initialize(m_Device, m_SwapChainImageFormat, m_DepthBuffer.GetDepthFormat());

	m_Texture.Initialize(m_Device, m_PhysicalDevice, m_CommandPool, m_GraphicsQueue, g_TexturePath1);

	m_Camera.Initialize(m_Device, m_PhysicalDevice, m_Window.GetAspectRatio(), m_Window.GetWindow());

	CreateFramebuffers();

	CreateGraphicsPipeline2D();
	CreateGraphicsPipeline3D();
	CreateScenes();

	m_SyncObjects.Initialize(m_Device);
}

void Application::MainLoop()
{
	while (!m_Window.WindowShouldClose())
	{
		Timer::Get().Update();
		m_Window.PollEvents();
		DrawFrame();
	}
	vkDeviceWaitIdle(m_Device);
}

void Application::Cleanup()
{
	CleanupSwapChain();

	m_Texture.Destroy(m_Device);

	m_Camera.Destroy(m_Device);

	m_GraphicsPipeline3D.Destroy(m_Device);
	m_GraphicsPipeline2D.Destroy(m_Device);

	m_RenderPass.Destroy(m_Device);

	m_SyncObjects.Destroy(m_Device);

	m_CommandPool.Destroy(m_Device);

	vkDestroyDevice(m_Device, nullptr);

	m_Surface.Destroy(m_VulkanInstance.GetVkInstance());

	m_VulkanInstance.Destroy();

	m_Window.Destroy();
}

void Application::DrawFrame()
{
	// Get Sync Objects
	const VkFence& inFlightFence{ m_SyncObjects.GetInFlightFence(m_CurrentFrame) };
	const VkSemaphore& imageAvailableSemaphore{ m_SyncObjects.GetImageAvailableSemaphore(m_CurrentFrame) };
	const VkSemaphore& renderFinishedSemaphore{ m_SyncObjects.GetRenderFinishedSemaphore(m_CurrentFrame) };

	// Wait for the fence from the previous frame
	vkWaitForFences(m_Device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

	// Acquire the next image from the swap chain
	uint32_t imageIndex{};
	VkResult acquireResult
	{
		vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX,
		imageAvailableSemaphore,
		VK_NULL_HANDLE, &imageIndex)
	};

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (acquireResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// Reset the fence for this frame
	vkResetFences(m_Device, 1, &inFlightFence);

	// Update the camera (view and projection matrices) uniform buffer
	m_Camera.Update(m_CurrentFrame);

	// Record commands in the command buffer for this frame
	RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

	// Submit the graphics command buffer
	VkSemaphore waitSemaphores[]{ imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[]{ renderFinishedSemaphore };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	m_CommandBuffers[m_CurrentFrame].Submit(submitInfo, m_GraphicsQueue, inFlightFence);

	// Present the swap chain image
	std::vector<VkSwapchainKHR> swapChains{ m_SwapChain };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	presentInfo.swapchainCount = static_cast<uint32_t>(swapChains.size());
	presentInfo.pSwapchains = swapChains.data();

	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	VkResult presentResult{ vkQueuePresentKHR(m_PresentQueue, &presentInfo) };

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || m_Window.GetFramebufferResized())
	{
		m_Window.SetFramebufferResized(false);
		RecreateSwapChain();
	}
	else if (presentResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image!");
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % g_MaxFramesInFlight;
}

void Application::RecordCommandBuffer(CommandBuffer commandBuffer, uint32_t imageIndex)
{
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass.GetVkRenderPass();
	renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapChainExtent.width);
	viewport.height = static_cast<float>(m_SwapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;

	const VkCommandBuffer& cmndBffr{ commandBuffer.GetVkCommandBuffer() };

	commandBuffer.Reset();

	commandBuffer.BeginRecording();

	commandBuffer.BeginRenderPass(renderPassInfo);

	vkCmdSetViewport(cmndBffr, 0, 1, &viewport);

	vkCmdSetScissor(cmndBffr, 0, 1, &scissor);

	m_GraphicsPipeline3D.Draw(cmndBffr, m_CurrentFrame);
	m_GraphicsPipeline2D.Draw(cmndBffr);

	commandBuffer.EndRenderPass();

	commandBuffer.EndRecording();
}

void Application::PickPhysicalDevice()
{
	uint32_t deviceCount{ 0 };
	vkEnumeratePhysicalDevices(m_VulkanInstance.GetVkInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices{ deviceCount };
	vkEnumeratePhysicalDevices(m_VulkanInstance.GetVkInstance(), &deviceCount, devices.data());

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			m_PhysicalDevice = device;
			break;
		}
	}
	if (m_PhysicalDevice == VK_NULL_HANDLE) throw std::runtime_error("failed to find a suitable GPU!");
}

void Application::CreateLogicDevice()
{
	QueueFamilyIndices indices{ FindQueueFamilies(m_PhysicalDevice) };

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

	createInfo.enabledExtensionCount = m_VulkanInstance.GetExtensionCount();
	createInfo.ppEnabledExtensionNames = m_VulkanInstance.GetExtionsionNames();

	if (m_VulkanInstance.GetValidationLayersEnabled())
	{
		createInfo.enabledLayerCount = m_VulkanInstance.GetValidationLayersCount();
		createInfo.ppEnabledLayerNames = m_VulkanInstance.GetValidationLayersNames();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
}

bool Application::IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices{ FindQueueFamilies(device) };
	bool extensionsSupported{ VulkanInstance::CheckDeviceExtensionSupport(device) };
	bool swapChainAdequate{ false };
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport{ QuerySwapChainSupport(device) };
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures{};
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices{};

	uint32_t queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int idx{ 0 };
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = idx;
		}

		VkBool32 presentSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, m_Surface.GetVkSurface(), &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = idx;
		}

		if (indices.IsComplete()) break;

		idx++;
	}

	return indices;
}

void Application::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport{ QuerySwapChainSupport(m_PhysicalDevice) };

	VkSurfaceFormatKHR surfaceFormat{ ChooseSwapSurfaceFormat(swapChainSupport.formats) };
	VkPresentModeKHR presentMode{ ChooseSwapPresentMode(swapChainSupport.presentModes) };
	VkExtent2D extent{ ChooseSwapExtent(swapChainSupport.capabilities) };

	uint32_t imageCount{ swapChainSupport.capabilities.minImageCount + 1 }; // recommended minimum + 1 (otherwise bottleneck)
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) // check to not exceed max
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	QueueFamilyIndices indices{ FindQueueFamilies(m_PhysicalDevice) };
	uint32_t queueFamilyIndices[]{ indices.graphicsFamily.value(), indices.presentFamily.value() };

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface.GetVkSurface();

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

	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr); // get only size
	m_SwapChainImages.resize(imageCount); // resize to correct size
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data()); // get swap chain image data

	// Store chosen format and extent
	m_SwapChainImageFormat = surfaceFormat.format; // VK_FORMAT_B8G8R8A8_SRGB
	m_SwapChainExtent = extent; // chosen resution of swap chain (width, height)
}

void Application::RecreateSwapChain()
{
	// Window minimization handling //
	int width{};
	int height{};
	m_Window.GetFramebufferSize(width, height);
	while (width == 0 || height == 0)
	{
		m_Window.GetFramebufferSize(width, height);
		m_Window.WaitEvents();
	}
	//////////////////////////////////

	vkDeviceWaitIdle(m_Device); // we shouldnt touch resources that may still be in use

	CleanupSwapChain();

	CreateSwapChain();
	CreateSwapChainImageViews();
	m_DepthBuffer.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, m_SwapChainExtent.width, m_SwapChainExtent.height);
	CreateFramebuffers();
}

void Application::CreateSwapChainImageViews()
{
	m_SwapChainImageViews.resize(m_SwapChainImages.size());
	for (size_t idx{}; idx < m_SwapChainImages.size(); ++idx)
	{
		ImageView imageView{};
		imageView.Initialize(m_Device, m_SwapChainImages[idx], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		m_SwapChainImageViews[idx] = imageView;
	}
}

void Application::CreateFramebuffers()
{
	m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

	for (size_t idx{}; idx < m_SwapChainImageViews.size(); idx++)
	{
		std::array<VkImageView, 2> attachments
		{
			m_SwapChainImageViews[idx].GetVkImageView(),
			m_DepthBuffer.GetVkImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass.GetVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_SwapChainExtent.width;
		framebufferInfo.height = m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Application::CleanupSwapChain()
{
	m_DepthBuffer.Destroy(m_Device);

	for (auto framebuffer : m_SwapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	for (auto imageview : m_SwapChainImageViews)
	{
		imageview.Destroy(m_Device);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice device)
{
	const VkSurfaceKHR surface{ m_Surface.GetVkSurface() };

	SwapChainSupportDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// Swap extent is the resolution of the swap chain images //

	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width;
		int height;
		m_Window.GetFramebufferSize(width, height);

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

void Application::CreateGraphicsPipeline2D()
{
	const VkRenderPass& renderPass{ m_RenderPass.GetVkRenderPass() };

	const ShaderConfig vertShaderConfig2D
	{
		"Shaders/shader2D.vert.spv",
		"main",
		VK_SHADER_STAGE_VERTEX_BIT
	};

	const ShaderConfig fragShaderConfig2D
	{
		"Shaders/shader2D.frag.spv",
		"main",
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	const ShadersConfigs shaderConfigs2D{ vertShaderConfig2D, fragShaderConfig2D };

	GraphicsPipelineConfigs configs{};
	configs.device = m_Device;
	configs.shaderConfigs = shaderConfigs2D;
	configs.swapchainExtent = m_SwapChainExtent;
	configs.renderPass = renderPass;

	m_GraphicsPipeline2D.Initialize(configs);
}

void Application::CreateGraphicsPipeline3D()
{
	const VkRenderPass& renderPass{ m_RenderPass.GetVkRenderPass() };

	const ShaderConfig vertShaderConfig3D
	{
		"Shaders/shader3D.vert.spv",
		"main",
		VK_SHADER_STAGE_VERTEX_BIT
	};

	const ShaderConfig fragShaderConfig3D
	{
		"Shaders/shader3D.frag.spv",
		"main",
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	const ShadersConfigs shaderConfigs3D{ vertShaderConfig3D, fragShaderConfig3D };

	GraphicsPipelineConfigs configs{};
	configs.device = m_Device;
	configs.shaderConfigs = shaderConfigs3D;
	configs.swapchainExtent = m_SwapChainExtent;
	configs.renderPass = renderPass;

	m_GraphicsPipeline3D.Initialize(configs, &m_Texture, &m_Camera);
}

void Application::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice) };
	m_CommandPool.Initialize(m_Device, queueFamilyIndices);
}

void Application::CreateCommandBuffers()
{
	m_CommandBuffers.resize(g_MaxFramesInFlight);

	for (int idx{}; idx < g_MaxFramesInFlight; ++idx)
	{
		m_CommandBuffers[idx] = m_CommandPool.CreateCommandBuffer(m_Device);
	}
}

void Application::CreateScenes()
{
	// 2D SCENE //
	std::vector<Model2D> models2D{};
	const std::vector<Vertex2D> vertices
	{
		{{-0.5f, -0.5f}, {0.5f, 0.2f, 0.8f}}, // Purple
		{{0.5f, -0.5f}, {0.8f, 0.6f, 0.2f}},  // Gold
		{{0.5f, 0.5f}, {0.2f, 0.8f, 0.5f}},   // Turquoise
		{{-0.5f, 0.5f}, {0.9f, 0.1f, 0.3f}}   // Red
	};
	const std::vector<uint32_t> indices
	{
		0, 1, 2, 2, 3, 0
	};
	Model2D model2D1{};
	model2D1.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, vertices, indices);
	model2D1.SetPosition(glm::vec2{ 0.f, 0.f });
	model2D1.SetRotation(90);
	model2D1.SetScale(glm::vec2{ 1.5f, 0.3f });

	models2D.emplace_back(std::move(model2D1));

	m_GraphicsPipeline2D.SetScene(std::move(models2D));

	// 3D SCENE //

	std::vector<Model3D> models3D{};

	Model3D model1{};
	model1.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model3DPath1);
	model1.SetPosition(glm::vec3{ -5.f, 0.5f, 0.f });

	Model3D model2{}; // plane
	model2.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_PlaneModel);
	model2.SetScale(50.f);

	Model3D model3{}; // cube
	model3.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model3DPath2);
	model3.SetPosition(glm::vec3{ 2.f, 2.f, 2.f });
	model3.SetScale(0.5f);

	Model3D model4{}; // cube
	model4.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model3DPath2);
	model4.SetPosition(glm::vec3{ -2.f, 2.f, 2.f });
	model4.SetScale(0.5f);

	Model3D model5{}; // cube
	model5.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model3DPath2);
	model5.SetPosition(glm::vec3{ 2.f, 2.f, -2.f });
	model5.SetScale(0.5f);

	Model3D model6{}; // cube
	model6.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model3DPath2);
	model6.SetPosition(glm::vec3{ -2.f, 2.f, -2.f });
	model6.SetScale(0.5f);

	models3D.emplace_back(std::move(model1));
	models3D.emplace_back(std::move(model2));
	models3D.emplace_back(std::move(model3));
	models3D.emplace_back(std::move(model4));
	models3D.emplace_back(std::move(model5));
	models3D.emplace_back(std::move(model6));

	m_GraphicsPipeline3D.SetScene(std::move(models3D));
}