#include "VulkanBase.h"

#include "Timer.h"

void VulkanBase::Run()
{
	InitVulkan();
	MainLoop();
	Cleanup();
}

void VulkanBase::InitVulkan()
{
	m_Window.Initialize();

	m_VulkanInstance.Initialize("VulkanEngine", "MorrogEngine", m_Window.GetWindow());
	m_Surface.Initialize(m_VulkanInstance.GetVkInstance(), m_Window.GetWindow());

	PickPhysicalDevice();
	CreateLogicDevice();

	CreateSwapChain();
	CreateSwapChainImageViews();
	
	m_RenderPass.Initialize(m_Device, m_SwapChainImageFormat, FindDepthFormat());

	CreateGraphicsPipeline3D();

	CreateCommandPool();

	CreateDepthResources();
	CreateFramebuffers();

	m_Texture.Initialize(m_Device, m_PhysicalDevice, m_CommandPool, m_GraphicsQueue, g_Texture1Path);
	CreateTextureSampler();

	CreateScene();
	m_Camera.Initialize(m_Device, m_PhysicalDevice, m_Window.GetAspectRatio(), m_Window.GetWindow());

	CreateDescriptorPool();
	AllocateDescriptorSets();
	UpdateDescriptorSets();

	CreateCommandBuffer();

	m_SyncObjects.Initialize(m_Device);
}

void VulkanBase::MainLoop()
{
	while (!m_Window.WindowShouldClose())
	{
		Timer::Get().Update();
		m_Window.PollEvents();
		DrawFrame();
	}
	vkDeviceWaitIdle(m_Device);
}

void VulkanBase::Cleanup()
{
	CleanupSwapChain();

	m_TextureSampler.Destroy(m_Device);
	m_Texture.Destroy(m_Device);

	m_Scene.Destroy(m_Device);

	m_Camera.Destroy(m_Device);
	
	vkFreeDescriptorSets(m_Device, m_DescriptorPool, static_cast<uint32_t>(m_DescriptorSets.size()), m_DescriptorSets.data());
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

	m_GraphicsPipeline3D.Destroy(m_Device);

	m_RenderPass.Destroy(m_Device);

	m_SyncObjects.Destroy(m_Device);

	m_CommandPool.Destroy(m_Device);

	vkDestroyDevice(m_Device, nullptr);

	m_Surface.Destroy(m_VulkanInstance.GetVkInstance());

	m_VulkanInstance.Destroy();

	m_Window.Destroy();
}

void VulkanBase::DrawFrame()
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

	// Update Uniform Buffers
	UpdateUniformBuffers();

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

void VulkanBase::PickPhysicalDevice()
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

void VulkanBase::CreateLogicDevice()
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

bool VulkanBase::IsDeviceSuitable(VkPhysicalDevice device)
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

QueueFamilyIndices VulkanBase::FindQueueFamilies(VkPhysicalDevice device)
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

void VulkanBase::CreateSwapChain()
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

void VulkanBase::RecreateSwapChain()
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
	CreateDepthResources();
	CreateFramebuffers();
}

void VulkanBase::CreateSwapChainImageViews()
{
	m_SwapChainImageViews.resize(m_SwapChainImages.size());
	for (size_t idx{}; idx < m_SwapChainImages.size(); ++idx)
	{
		ImageView imageView{};
		imageView.Initialize(m_Device, m_SwapChainImages[idx], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		m_SwapChainImageViews[idx] = imageView;
	}
}

void VulkanBase::CreateFramebuffers()
{
	m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

	for (size_t idx{}; idx < m_SwapChainImageViews.size(); idx++)
	{
		std::array<VkImageView, 2> attachments
		{
			m_SwapChainImageViews[idx].GetVkImageView(),
			m_DepthImageView.GetVkImageView()
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

void VulkanBase::CleanupSwapChain()
{
	m_DepthImage.Destroy(m_Device);
	m_DepthImageView.Destroy(m_Device);

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

SwapChainSupportDetails VulkanBase::QuerySwapChainSupport(VkPhysicalDevice device)
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

VkSurfaceFormatKHR VulkanBase::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR VulkanBase::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D VulkanBase::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

void VulkanBase::CreateGraphicsPipeline3D()
{
	const ShaderConfig vertShaderConfig
	{
		"Shaders/shader3D.vert.spv",
		"main",
		VK_SHADER_STAGE_VERTEX_BIT
	};

	const ShaderConfig fragShaderConfig
	{
		"Shaders/shader3D.frag.spv",
		"main",
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	m_GraphicsPipeline3D.Initialize(m_Device, vertShaderConfig, fragShaderConfig, m_SwapChainExtent, m_RenderPass.GetVkRenderPass());
}

void VulkanBase::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices{ FindQueueFamilies(m_PhysicalDevice) };
	m_CommandPool.Initialize(m_Device, queueFamilyIndices);
}

void VulkanBase::CreateCommandBuffer()
{
	m_CommandBuffers.resize(g_MaxFramesInFlight);

	for (int idx{}; idx < g_MaxFramesInFlight; ++idx)
	{
		m_CommandBuffers[idx] = m_CommandPool.CreateCommandBuffer(m_Device);
	}
}

void VulkanBase::RecordCommandBuffer(CommandBuffer commandBuffer, uint32_t imageIndex)
{
	constexpr VkPipelineBindPoint bindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS };

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

	commandBuffer.Reset();

	commandBuffer.BeginRecording();

	VkCommandBuffer cmndBffr{ commandBuffer.GetVkCommandBuffer() };

	commandBuffer.BeginRenderPass(renderPassInfo);
	{
		vkCmdSetViewport(cmndBffr, 0, 1, &viewport);

		vkCmdSetScissor(cmndBffr, 0, 1, &scissor);

		m_GraphicsPipeline3D.Draw(cmndBffr, m_DescriptorSets[m_CurrentFrame], m_CurrentFrame);
	}
	commandBuffer.EndRenderPass();

	commandBuffer.EndRecording();
}

void VulkanBase::CreateDescriptorPool()
{
	const size_t nrOfModels{ m_GraphicsPipeline3D.GetNrOfModels() };
	const size_t totalDescriptorSets{ g_MaxFramesInFlight * (nrOfModels + 1) }; // +1 for camera

	std::array<VkDescriptorPoolSize, 3> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Model Matrix glm::mat4
	poolSizes[0].descriptorCount = static_cast<uint32_t>(g_MaxFramesInFlight * nrOfModels);

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Sampler
	poolSizes[1].descriptorCount = static_cast<uint32_t>(g_MaxFramesInFlight);

	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Camera uniform buffer
	poolSizes[2].descriptorCount = static_cast<uint32_t>(g_MaxFramesInFlight);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(totalDescriptorSets);
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanBase::AllocateDescriptorSets()
{
	const size_t nrOfModels{ m_GraphicsPipeline3D.GetNrOfModels() };
	const size_t totalDescriptorSets{ g_MaxFramesInFlight * (nrOfModels + 1) }; // +1 for camera

	std::vector<VkDescriptorSetLayout> layouts{ totalDescriptorSets, m_GraphicsPipeline3D.GetDescriptorSetLayout() };
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(totalDescriptorSets);
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(totalDescriptorSets);

	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

void VulkanBase::UpdateDescriptorSets()
{
	const std::vector<Model3D>& models{ m_Scene.GetModels() };
	const size_t nrOfModels{ models.size() };
	const std::vector<DataBuffer> cameraBuffers{ m_Camera.GetUniformBuffers() };

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = m_Texture.GetImageView().GetVkImageView();
	imageInfo.sampler = m_TextureSampler.GetVkSampler();

	for (size_t frameIdx{}; frameIdx < g_MaxFramesInFlight; ++frameIdx)
	{
		// Update descriptor set for the camera
		VkDescriptorBufferInfo cameraBufferInfo{};
		cameraBufferInfo.buffer = cameraBuffers[frameIdx].GetVkBuffer();
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = sizeof(CameraUBO);

		VkWriteDescriptorSet cameraDescriptorWrite[2]{};
		cameraDescriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraDescriptorWrite[0].dstSet = m_DescriptorSets[frameIdx];
		cameraDescriptorWrite[0].dstBinding = 0;
		cameraDescriptorWrite[0].dstArrayElement = 0;
		cameraDescriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDescriptorWrite[0].descriptorCount = 1;
		cameraDescriptorWrite[0].pBufferInfo = &cameraBufferInfo;

		cameraDescriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraDescriptorWrite[1].dstSet = m_DescriptorSets[frameIdx];
		cameraDescriptorWrite[1].dstBinding = 1;
		cameraDescriptorWrite[1].dstArrayElement = 0;
		cameraDescriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		cameraDescriptorWrite[1].descriptorCount = 1;
		cameraDescriptorWrite[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device, 2, cameraDescriptorWrite, 0, nullptr);

		// Update descriptor sets for each model
		for (size_t modelIdx{}; modelIdx < nrOfModels; ++modelIdx)
		{
			const std::vector<DataBuffer>& modelBuffers{ models[modelIdx].GetBuffers() };

			VkDescriptorBufferInfo modelBufferInfo{};
			modelBufferInfo.buffer = modelBuffers[frameIdx].GetVkBuffer();
			modelBufferInfo.offset = 0;
			modelBufferInfo.range = sizeof(ModelUBO);

			VkWriteDescriptorSet descriptorWrites{};
			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = m_DescriptorSets[frameIdx * nrOfModels + modelIdx + 1]; // +1 for camera
			descriptorWrites.dstBinding = 0;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pBufferInfo = &modelBufferInfo;

			vkUpdateDescriptorSets(m_Device, 1, &descriptorWrites, 0, nullptr);
		}
	}
}

void VulkanBase::CreateTextureSampler()
{
	m_TextureSampler.Initialize(m_Device, m_PhysicalDevice);
}

void VulkanBase::CreateScene()
{
	//std::vector<Model3D> models{};

	//Model3D model1{};
	//model1.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model1Path);
	//model1.SetPosition(glm::vec3{ -5.f, 0.5f, 0.f });

	//Model3D model2{}; // plane
	//model2.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model3Path);
	//model2.SetScale(50.f);

	//Model3D model3{}; // cube
	//model3.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model2Path);
	//model3.SetPosition(glm::vec3{ 2.f, 2.f, 2.f });
	//model3.SetScale(0.5f);

	//Model3D model4{}; // cube
	//model4.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model2Path);
	//model4.SetPosition(glm::vec3{ -2.f, 2.f, 2.f });
	//model4.SetScale(0.5f);

	//Model3D model5{}; // cube
	//model5.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model2Path);
	//model5.SetPosition(glm::vec3{ 2.f, 2.f, -2.f });
	//model5.SetScale(0.5f);

	//Model3D model6{}; // cube
	//model6.Initialize(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, g_Model2Path);
	//model6.SetPosition(glm::vec3{ -2.f, 2.f, -2.f });
	//model6.SetScale(0.5f);

	//models.emplace_back(std::move(model1));
	//models.emplace_back(std::move(model2));
	//models.emplace_back(std::move(model3));
	//models.emplace_back(std::move(model4));
	//models.emplace_back(std::move(model5));
	//models.emplace_back(std::move(model6));

	//m_Scene.Initialize(std::move(models));

	m_GraphicsPipeline3D.InitScene(m_Device, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool);
}

void VulkanBase::UpdateUniformBuffers()
{
	// Update the camera (view and projection matrices) uniform buffer
	m_Camera.Update(m_CurrentFrame);
}

void VulkanBase::CreateDepthResources()
{
	VkFormat depthFormat{ FindDepthFormat() };

	m_DepthImage.Initialize(m_Device, m_PhysicalDevice, m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	m_DepthImageView.Initialize(m_Device, m_DepthImage.GetVkImage(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	m_DepthImage.TransitionImageLayout(m_Device, m_CommandPool, m_GraphicsQueue, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat VulkanBase::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) return format;
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanBase::FindDepthFormat()
{
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool VulkanBase::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}