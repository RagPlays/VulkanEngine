#include <stdexcept>

#include "GraphicsPipeline3DIR.h"

#include "VulkanUtils.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"

void GraphicsPipeline3DIR::Initialize(const GraphicsPipelineConfigs& configs, const Texture& tex, const Camera& cam)
{
	CreateDescriptorSetLayout(configs.device);
	CreateDescriptorPool(configs.device);
	AllocateDescriptorSets(configs.device);
	UpdateDescriptorSets(configs.device, tex, cam);

	CreatePipelineLayout(configs.device);
	CreatePipeline(configs.device, configs.shaderConfigs, configs.swapchainExtent, configs.renderPass);
}

void GraphicsPipeline3DIR::Destory(VkDevice device)
{
	vkDeviceWaitIdle(device);

	if (m_VkPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device, m_VkPipelineLayout, VK_NULL_HANDLE);
		m_VkPipelineLayout = VK_NULL_HANDLE;
	}

	if (m_VkPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device, m_VkPipeline, VK_NULL_HANDLE);
		m_VkPipeline = VK_NULL_HANDLE;
	}

	if (m_DescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device, m_DescriptorPool, VK_NULL_HANDLE);
		m_DescriptorPool = VK_NULL_HANDLE;
	}

	if (m_VkDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, m_VkDescriptorSetLayout, VK_NULL_HANDLE);
		m_VkDescriptorSetLayout = VK_NULL_HANDLE;
	}

	m_Scene.Destroy(device);
}

void GraphicsPipeline3DIR::Update(VkDevice device)
{
	m_Scene.Update(device);
}

void GraphicsPipeline3DIR::Draw(VkCommandBuffer commandBuffer, uint32_t currentFrame) const
{
	constexpr VkPipelineBindPoint bindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS };

	vkCmdBindPipeline(commandBuffer, bindPoint, m_VkPipeline);

	vkCmdBindDescriptorSets(commandBuffer, bindPoint, m_VkPipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, VK_NULL_HANDLE);

	m_Scene.Draw(commandBuffer, m_VkPipelineLayout);
}

void GraphicsPipeline3DIR::SetScene(Scene3DIR&& scene)
{
	m_Scene = std::move(scene);
}

void GraphicsPipeline3DIR::SetScene(std::vector<Model3DIR>&& models)
{
	m_Scene.Initialize(std::move(models));
}

void GraphicsPipeline3DIR::CreateDescriptorSetLayout(VkDevice device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = VK_NULL_HANDLE; // Optional
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = VK_NULL_HANDLE;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings{ uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &m_VkDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create descriptor set layout!" };
	}
}

void GraphicsPipeline3DIR::CreateDescriptorPool(VkDevice device)
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Camera uniform buffer
	poolSizes[0].descriptorCount = static_cast<uint32_t>(g_MaxFramesInFlight);

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Sampler
	poolSizes[1].descriptorCount = static_cast<uint32_t>(g_MaxFramesInFlight);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(g_MaxFramesInFlight);
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create descriptor pool!" };
	}
}

void GraphicsPipeline3DIR::AllocateDescriptorSets(VkDevice device)
{
	std::vector<VkDescriptorSetLayout> layouts{ g_MaxFramesInFlight, m_VkDescriptorSetLayout };

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(layouts.size());

	if (vkAllocateDescriptorSets(device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to allocate descriptor sets!" };
	}
}

void GraphicsPipeline3DIR::UpdateDescriptorSets(VkDevice device, const Texture& tex, const Camera& cam)
{
	const auto& cameraBuffers{ cam.GetUniformBuffers() };

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = tex.GetVkImageView();
	imageInfo.sampler = tex.GetVkSampler();

	for (size_t frameIdx{}; frameIdx < g_MaxFramesInFlight; ++frameIdx)
	{
		VkDescriptorBufferInfo cameraBufferInfo{};
		cameraBufferInfo.buffer = cameraBuffers[frameIdx].GetVkBuffer();
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = sizeof(CameraUBO);

		std::array<VkWriteDescriptorSet, 2>descriptorWrites{};

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_DescriptorSets[frameIdx];
		descriptorWrites[1].dstBinding = 0;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &cameraBufferInfo;

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[frameIdx];
		descriptorWrites[0].dstBinding = 1;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VK_NULL_HANDLE);
	}
}

void GraphicsPipeline3DIR::CreatePipelineLayout(VkDevice device)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_VkDescriptorSetLayout;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, VK_NULL_HANDLE, &m_VkPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create pipeline layout!" };
	}
}

void GraphicsPipeline3DIR::CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass)
{
	Shader vertShader{};
	Shader fragShader{};
	vertShader.Initialize(device, shaderConfigs.vertShaderConfig);
	fragShader.Initialize(device, shaderConfigs.fragShaderConfig);

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages
	{
		vertShader.GetPipelineShaderStageInfo(),
		fragShader.GetPipelineShaderStageInfo()
	};

	const auto bindingDescription{ Descriptions::Get3DIRBindingDescriptions() };
	const auto attributeDescriptions{ Descriptions::Get3DIRAttributeDescriptions() };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	const auto inputAssembly{ Shader::GetInputAssemblyStateInfo() };

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = VK_NULL_HANDLE;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = VK_NULL_HANDLE;
	pipelineInfo.layout = m_VkPipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VkPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create graphics pipeline!" };
	}

	vertShader.Destroy(device);
	fragShader.Destroy(device);
}