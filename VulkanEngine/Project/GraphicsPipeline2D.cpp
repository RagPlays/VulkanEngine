#include <stdexcept>

#include "GraphicsPipeline2D.h"

#include "Shader.h"
#include "VulkanStructs.h"

void GraphicsPipeline2D::Initialize(const GraphicsPipelineConfigs& configs)
{
	CreatePipelineLayout(configs.device);
	CreatePipeline(configs.device, configs.shaderConfigs, configs.swapchainExtent, configs.renderPass);
}

void GraphicsPipeline2D::Destroy(VkDevice device)
{
	m_Scene.Destroy(device);

	vkDestroyPipeline(device, m_VkPipeline, nullptr);
	vkDestroyPipelineLayout(device, m_VkPipelineLayout, nullptr);
}

void GraphicsPipeline2D::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipeline);

	m_Scene.Draw(commandBuffer, m_VkPipelineLayout);
}

void GraphicsPipeline2D::SetScene(std::vector<Model2D>&& models)
{
	m_Scene.Initialize(std::move(models));
}

void GraphicsPipeline2D::CreatePipelineLayout(VkDevice device)
{
	std::array<VkPushConstantRange, 1> pushConstantRange{};
	pushConstantRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange[0].offset = 0;
	pushConstantRange[0].size = sizeof(ModelUBO);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRange.size());
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRange.data();

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
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

	auto bindingDescription{ std::array<VkVertexInputBindingDescription, 1>{ Vertex2D::GetBindingDescription() } };
	auto attributeDescriptions{ std::array<VkVertexInputAttributeDescription, 2>{ Vertex2D::GetAttributeDescriptions() } };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	auto inputAssembly{ Shader::GetInputAssemblyStateInfo() };

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
	viewport[0].x = 0.0f;
	viewport[0].y = 0.0f;
	viewport[0].width = static_cast<float>(swapchainExtent.width);
	viewport[0].height = static_cast<float>(swapchainExtent.height);
	viewport[0].minDepth = 0.0f;
	viewport[0].maxDepth = 1.0f;

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
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
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
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_ALWAYS;
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
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vertShader.Destroy(device);
	fragShader.Destroy(device);
}