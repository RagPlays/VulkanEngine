#include <stdexcept>

#include "GraphicsPipeline2D.h"

#include "Shader.h"
#include "VulkanUtils.h"
#include "VulkanStructs.h"
#include "Camera.h"

void GraphicsPipeline2D::Initialize(const GraphicsPipelineConfigs& configs, const Camera& pCamera)
{
	CreateDescriptorSetLayout(configs.device);
	CreateDescriptorPool(configs.device);
	AllocateDescriptorSets(configs.device);
	UpdateDescriptorSets(configs.device, pCamera);

	CreatePipelineLayout(configs.device);
	CreatePipeline(configs.device, configs.shaderConfigs, configs.swapchainExtent, configs.renderPass);
}

void GraphicsPipeline2D::Destroy(VkDevice device)
{
	if (m_VkPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device, m_VkPipelineLayout, nullptr);
		m_VkPipelineLayout = VK_NULL_HANDLE;
	}

	if (m_VkPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device, m_VkPipeline, nullptr);
		m_VkPipeline = VK_NULL_HANDLE;
	}

	if (m_DescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
		m_DescriptorPool = VK_NULL_HANDLE;
	}

	if (m_VkDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device, m_VkDescriptorSetLayout, nullptr);
		m_VkDescriptorSetLayout = VK_NULL_HANDLE;
	}

	m_Scene.Destroy(device);
}

void GraphicsPipeline2D::Draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipelineLayout, 0, 1, &m_DescriptorSets[currentFrame], 0, nullptr);

	m_Scene.Draw(commandBuffer, m_VkPipelineLayout);
}

void GraphicsPipeline2D::SetScene(std::vector<Model2D>&& models)
{
	m_Scene.Initialize(std::move(models));
}

void GraphicsPipeline2D::CreateDescriptorSetLayout(VkDevice device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = VK_NULL_HANDLE; // Optional
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings{ uboLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &m_VkDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create descriptor set layout!" };
	}
}

void GraphicsPipeline2D::CreateDescriptorPool(VkDevice device)
{
	std::array<VkDescriptorPoolSize, 1> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Camera uniform buffer
	poolSizes[0].descriptorCount = static_cast<uint32_t>(g_MaxFramesInFlight);

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

void GraphicsPipeline2D::AllocateDescriptorSets(VkDevice device)
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

void GraphicsPipeline2D::UpdateDescriptorSets(VkDevice device, const Camera& pCam)
{
	const std::vector<DataBuffer>& cameraBuffers{ pCam.GetUniformBuffers() };

	for (size_t frameIdx{}; frameIdx < g_MaxFramesInFlight; ++frameIdx)
	{
		VkDescriptorBufferInfo cameraBufferInfo{};
		cameraBufferInfo.buffer = cameraBuffers[frameIdx].GetVkBuffer();
		cameraBufferInfo.offset = 0;
		cameraBufferInfo.range = sizeof(CameraUBO);

		std::array<VkWriteDescriptorSet, 1>descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[frameIdx];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &cameraBufferInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VK_NULL_HANDLE);
	}
}

void GraphicsPipeline2D::CreatePipelineLayout(VkDevice device)
{
	std::array<VkPushConstantRange, 1> pushConstantRange{};
	pushConstantRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange[0].offset = 0;
	pushConstantRange[0].size = sizeof(ModelUBO);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_VkDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRange.data();

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create pipeline layout!" };
	}
}

void GraphicsPipeline2D::CreatePipeline(VkDevice device, const ShadersConfigs& shaderConfigs, const VkExtent2D& swapchainExtent, VkRenderPass renderPass)
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

	const auto bindingDescription{ Descriptions::Get2DBindingDescriptions() };
	const auto attributeDescriptions{ Descriptions::Get2DAttributeDescriptions() };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	const auto inputAssembly{ Shader::GetInputAssemblyStateInfo() };

	std::array<VkDynamicState, 2> dynamicStates
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	std::array<VkViewport, 1> viewport{};
	viewport[0].x = 0.f;
	viewport[0].y = 0.f;
	viewport[0].width = static_cast<float>(swapchainExtent.width);
	viewport[0].height = static_cast<float>(swapchainExtent.height);
	viewport[0].minDepth = 0.f;
	viewport[0].maxDepth = 1.f;

	std::array<VkRect2D, 1> scissor{};
	scissor[0].offset = { 0, 0 };
	scissor[0].extent = swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = static_cast<uint32_t>(viewport.size());
	viewportState.pViewports = viewport.data();
	viewportState.scissorCount = static_cast<uint32_t>(scissor.size());
	viewportState.pScissors = scissor.data();

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// VK_POLYGON_MODE_FILL (fill the area of the polygon with fragments)
	// VK_POLYGON_MODE_LINE (polygon edges are drawn as lines)
	// VK_POLYGON_MODE_POINT (polygon vertices are drawn as points)
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // off for 2D
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.f; // Optional
	multisampling.pSampleMask = VK_NULL_HANDLE; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.f; // Optional
	colorBlending.blendConstants[1] = 0.f; // Optional
	colorBlending.blendConstants[2] = 0.f; // Optional
	colorBlending.blendConstants[3] = 0.f; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = m_VkPipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VkPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error{ "failed to create graphics pipeline!" };
	}

	vertShader.Destroy(device);
	fragShader.Destroy(device);
}