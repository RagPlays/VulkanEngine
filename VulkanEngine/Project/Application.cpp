#include "Application.h"
#include "Timer.h"

#define USE_DEBUG_BACKGROUND_COLOR

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

	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };
	const VkPhysicalDevice& phyDevice{ m_VulkanInstance.GetVkPhysicalDevice() };
	const VkQueue& graphQ{ m_VulkanInstance.GetGraphicsQueue() };

	m_Swapchain.Initialize(m_VulkanInstance, m_Window);

	m_CommandPool.Initialize(m_VulkanInstance);
	CreateCommandBuffers();

	m_DepthBuffer.Initialize(m_VulkanInstance, m_CommandPool, m_Swapchain);

	m_RenderPass.Initialize(m_VulkanInstance, m_Swapchain, m_DepthBuffer);

	CreateFramebuffers();

	m_Texture.Initialize(m_VulkanInstance, m_CommandPool, g_TexturePath1);

	m_Camera.Initialize(m_VulkanInstance, m_Window);

	CreateGraphicsPipeline2D();
	CreateGraphicsPipeline3D();
	CreateGraphicsPipeline3DIR();

	Create2DScene();
	Create3DScene();
	Create3DIRScene();

	m_SyncObjects.Initialize(device);
}

void Application::MainLoop()
{
	while (!m_Window.WindowShouldClose())
	{
		Timer::Get().Update();
		m_Window.PollEvents();
		Update();
		DrawFrame();
	}
	m_VulkanInstance.DeviceWaitIdle();
}

void Application::Cleanup()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };

	CleanupWindowResources();

	m_Texture.Destroy(device);

	m_Camera.Destroy(device);

	m_GraphicsPipeline3DIR.Destory(device);
	m_GraphicsPipeline3D.Destroy(device);
	m_GraphicsPipeline2D.Destroy(device);

	m_RenderPass.Destroy(device);

	m_SyncObjects.Destroy(device);

	m_CommandPool.Destroy(device);

	m_VulkanInstance.Destroy();

	m_Window.Destroy();
}

void Application::Update()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };

	// Update the camera (view and projection matrices) uniform buffer
	m_Camera.Update(device, m_CurrentFrame);

	// Update models
	m_GraphicsPipeline3DIR.Update(m_VulkanInstance.GetVkDevice());
}

void Application::DrawFrame()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };
	const VkQueue& graphQ{ m_VulkanInstance.GetGraphicsQueue() };
	const VkQueue& presQ{ m_VulkanInstance.GetPresentQueue() };

	// Get Sync Objects
	const VkFence& inFlightFence{ m_SyncObjects.GetInFlightFence(m_CurrentFrame) };
	const VkSemaphore& imageAvailableSemaphore{ m_SyncObjects.GetImageAvailableSemaphore(m_CurrentFrame) };
	const VkSemaphore& renderFinishedSemaphore{ m_SyncObjects.GetRenderFinishedSemaphore(m_CurrentFrame) };

	// Wait for the fence from the previous frame
	vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

	// Acquire the next image from the swap chain
	uint32_t imageIndex{};
	const VkResult acquireResult
	{
		vkAcquireNextImageKHR(device, m_Swapchain.GetVkSwapchain(), UINT64_MAX,
		imageAvailableSemaphore,
		VK_NULL_HANDLE, &imageIndex)
	};

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
	{
		RecreateWindowResources();
		return;
	}
	else if (acquireResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// Reset the fence for this frame
	vkResetFences(device, 1, &inFlightFence);

	// Record commands in the command buffer for this frame
	RecordCommandBuffer(imageIndex);

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

	m_CommandBuffers[m_CurrentFrame].Submit(submitInfo, graphQ, inFlightFence);

	// Present the swap chain image
	std::array<VkSwapchainKHR, 1> swapChains{ m_Swapchain.GetVkSwapchain() };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	presentInfo.swapchainCount = static_cast<uint32_t>(swapChains.size());
	presentInfo.pSwapchains = swapChains.data();

	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	const VkResult presentResult{ vkQueuePresentKHR(presQ, &presentInfo) };

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || m_Window.GetFramebufferResized())
	{
		m_Window.SetFramebufferResized(false);
		RecreateWindowResources();
	}
	else if (presentResult != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swap chain image!");
	}

	m_CurrentFrame = (m_CurrentFrame + 1) % g_MaxFramesInFlight;
}

void Application::RecordCommandBuffer(uint32_t imageIndex)
{
	const VkExtent2D& swapchainExtent{ m_Swapchain.GetVkExtent() };
	const CommandBuffer& comndBffr{ m_CommandBuffers[m_CurrentFrame] };
	const VkCommandBuffer& VkCmndBffr{ comndBffr.GetVkCommandBuffer() };

	const glm::vec3& cameraDir{ glm::normalize(m_Camera.GetDirection()) };

#ifdef USE_DEBUG_BACKGROUND_COLOR

	VkClearColorValue clearColor
	{
		{ cameraDir.x, cameraDir.y, cameraDir.z, 1.f }
	};

#else

	VkClearColorValue clearColor
	{
		{ 0.025f, 0.025f, 0.025f, 1.f }
	};

#endif

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_RenderPass.GetVkRenderPass();
	renderPassInfo.framebuffer = m_FrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapchainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	VkViewport viewport{};
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = static_cast<float>(swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainExtent;

	comndBffr.Reset();

	comndBffr.BeginRecording();

	comndBffr.BeginRenderPass(renderPassInfo);
	{
		vkCmdSetViewport(VkCmndBffr, 0, 1, &viewport);

		vkCmdSetScissor(VkCmndBffr, 0, 1, &scissor);

		m_GraphicsPipeline3DIR.Draw(VkCmndBffr, m_CurrentFrame);
		m_GraphicsPipeline3D.Draw(VkCmndBffr, m_CurrentFrame);
		m_GraphicsPipeline2D.Draw(VkCmndBffr, m_CurrentFrame);
	}
	comndBffr.EndRenderPass();

	comndBffr.EndRecording();
}

void Application::CleanupWindowResources()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };

	m_DepthBuffer.Destroy(device);

	for (auto framebuffer : m_FrameBuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	m_Swapchain.Destroy(device);
}

void Application::RecreateWindowResources()
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

	m_VulkanInstance.DeviceWaitIdle(); // we shouldnt touch resources that may still be in use

	CleanupWindowResources();

	m_Swapchain.Initialize(m_VulkanInstance, m_Window);

	m_DepthBuffer.Initialize(m_VulkanInstance, m_CommandPool, m_Swapchain);
	CreateFramebuffers();
}

void Application::CreateFramebuffers()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };
	const std::vector<ImageView>& swapchainImageViews{ m_Swapchain.GetImageViews() };
	const VkExtent2D& swapchainExtent{ m_Swapchain.GetVkExtent() };

	m_FrameBuffers.resize(swapchainImageViews.size());

	for (size_t idx{}; idx < swapchainImageViews.size(); idx++)
	{
		const std::array<VkImageView, 2> attachments
		{
			swapchainImageViews[idx].GetVkImageView(),
			m_DepthBuffer.GetVkImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass.GetVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_FrameBuffers[idx]) != VK_SUCCESS)
		{
			throw std::runtime_error{ "failed to create framebuffer!" };
		}
	}
}

void Application::CreateGraphicsPipeline2D()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };
	const VkRenderPass& renderPass{ m_RenderPass.GetVkRenderPass() };
	const VkExtent2D& swapchainExtent{ m_Swapchain.GetVkExtent() };

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
	configs.device = device;
	configs.shaderConfigs = shaderConfigs2D;
	configs.swapchainExtent = swapchainExtent;
	configs.renderPass = renderPass;

	m_GraphicsPipeline2D.Initialize(configs, m_Camera);
}

void Application::CreateGraphicsPipeline3D()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };
	const VkRenderPass& renderPass{ m_RenderPass.GetVkRenderPass() };
	const VkExtent2D& swapchainExtent{ m_Swapchain.GetVkExtent() };

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
	configs.device = device;
	configs.shaderConfigs = shaderConfigs3D;
	configs.swapchainExtent = swapchainExtent;
	configs.renderPass = renderPass;

	m_GraphicsPipeline3D.Initialize(configs, m_Texture, m_Camera);
}

void Application::CreateGraphicsPipeline3DIR()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };
	const VkRenderPass& renderPass{ m_RenderPass.GetVkRenderPass() };
	const VkExtent2D& swapchainExtent{ m_Swapchain.GetVkExtent() };

	const ShaderConfig vertShaderConfig3D
	{
		"Shaders/shader3DIR.vert.spv",
		"main",
		VK_SHADER_STAGE_VERTEX_BIT
	};

	const ShaderConfig fragShaderConfig3D
	{
		"Shaders/shader3DIR.frag.spv",
		"main",
		VK_SHADER_STAGE_FRAGMENT_BIT
	};

	const ShadersConfigs shaderConfigs3D{ vertShaderConfig3D, fragShaderConfig3D };

	GraphicsPipelineConfigs configs{};
	configs.device = device;
	configs.shaderConfigs = shaderConfigs3D;
	configs.swapchainExtent = swapchainExtent;
	configs.renderPass = renderPass;

	m_GraphicsPipeline3DIR.Initialize(configs, m_Camera);
}

void Application::CreateCommandBuffers()
{
	const VkDevice& device{ m_VulkanInstance.GetVkDevice() };

	m_CommandBuffers.resize(g_MaxFramesInFlight);

	for (int idx{}; idx < g_MaxFramesInFlight; ++idx)
	{
		m_CommandBuffers[idx] = m_CommandPool.CreateCommandBuffer(device);
	}
}

void Application::Create2DScene()
{
	std::vector<Model2D> sceneModels{};

	// Square
	std::vector<Vertex2D> vertices
	{
		{{-0.5f, -0.5f}, {0.5f, 0.2f, 0.8f}}, // Purple
		{{0.5f, -0.5f}, {0.8f, 0.6f, 0.2f}},  // Gold
		{{0.5f, 0.5f}, {0.2f, 0.8f, 0.5f}},   // Turquoise
		{{-0.5f, 0.5f}, {0.9f, 0.1f, 0.3f}}   // Red
	};
	std::vector<uint32_t> indices
	{
		0, 1, 2, 2, 3, 0
	};
	Model2D model1{};
	model1.Initialize(m_VulkanInstance, m_CommandPool, vertices, indices);

	// Triangle
	vertices =
	{
		{{0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}},  // Top, Red
		{{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},  // Bottom Left, Green
		{{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}}   // Bottom Right, Blue
	};
	indices = 
	{
		0, 1, 2
	};
	Model2D model2{};
	model2.Initialize(m_VulkanInstance, m_CommandPool, vertices, indices);

	// Pentagon
	vertices =
	{
		{{0.0f, 0.5f}, {0.7f, 0.2f, 0.2f}},     // Top vertex, Light Red
		{{0.47f, 0.15f}, {0.2f, 0.7f, 0.2f}},   // Top right vertex, Light Green
		{{0.29f, -0.4f}, {0.2f, 0.2f, 0.7f}},   // Bottom right vertex, Light Blue
		{{-0.29f, -0.4f}, {0.7f, 0.7f, 0.2f}},  // Bottom left vertex, Light Yellow
		{{-0.47f, 0.15f}, {0.7f, 0.2f, 0.7f}}   // Top left vertex, Light Purple
	};
	indices =
	{
		0, 1, 2,
		0, 2, 3,
		0, 3, 4
	};
	Model2D model3{};
	model3.Initialize(m_VulkanInstance, m_CommandPool, vertices, indices);
	
	model1.SetPosition(glm::vec2{ -0.65f, -0.33f });
	model1.SetRotation(0);
	model1.SetScale(0.1f);

	model2.SetPosition(glm::vec2{ -0.5f, -0.33f });
	model2.SetRotation(0);
	model2.SetScale(0.1f);

	model3.SetPosition(glm::vec2{ -0.35f, -0.33f });
	model3.SetRotation(0);
	model3.SetScale(0.12f);

	sceneModels.emplace_back(std::move(model1));
	sceneModels.emplace_back(std::move(model2));
	sceneModels.emplace_back(std::move(model3));

	m_GraphicsPipeline2D.SetScene(std::move(sceneModels));
}

void Application::Create3DScene()
{
	std::vector<Model3D> sceneModels{};

	// cube
	Model3D model1{};
	model1.Initialize(m_VulkanInstance, m_CommandPool, g_CubeModel);

	// plane
	Model3D model2{};
	model2.Initialize(m_VulkanInstance, m_CommandPool, g_PlaneModel);

	Model3D model3{};
	model3.Initialize(m_VulkanInstance, m_CommandPool, g_Model3DPath1);
	Model3D model4{};
	model4.Initialize(m_VulkanInstance, m_CommandPool, g_Model3DPath1);
	Model3D model5{};
	model5.Initialize(m_VulkanInstance, m_CommandPool, g_Model3DPath1);
	Model3D model6{};
	model6.Initialize(m_VulkanInstance, m_CommandPool, g_Model3DPath1);

	// Translations
	model1.SetPosition(glm::vec3{ -5.f, 0.5f, 0.f });
	model1.SetScale(0.5f);

	model2.SetScale(50.f);

	model3.SetPosition(glm::vec3{ 2.f, 2.f, 2.f });

	model4.SetPosition(glm::vec3{ -2.f, 2.f, 2.f });

	model5.SetPosition(glm::vec3{ 2.f, 2.f, -2.f });

	model6.SetPosition(glm::vec3{ -2.f, 2.f, -2.f });

	// adding models
	sceneModels.emplace_back(std::move(model1));
	sceneModels.emplace_back(std::move(model2));
	sceneModels.emplace_back(std::move(model3));
	sceneModels.emplace_back(std::move(model4));
	sceneModels.emplace_back(std::move(model5));
	sceneModels.emplace_back(std::move(model6));

	m_GraphicsPipeline3D.SetScene(std::move(sceneModels));
}

void Application::Create3DIRScene()
{
	std::vector<Model3DIR> sceneModels{};

	// cube
	Model3DIR cube{};
	cube.Initialize(m_VulkanInstance, m_CommandPool, g_CubeModel, 2);
	cube.SetPosition(0, glm::vec3{ -2.f, 0.5f, 0.f });
	cube.SetScale(0, 0.5f);

	cube.SetPosition(1, glm::vec3{ 2.f, 0.5f, 0.f });
	cube.SetScale(1, 0.5f);

	sceneModels.emplace_back(std::move(cube));

	m_GraphicsPipeline3DIR.SetScene(std::move(sceneModels));
}