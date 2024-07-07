#include <stdexcept>

#include "GraphicsPipeline3D.h"

#include "Shader.h"
#include "VulkanUtils.h"

void GraphicsPipeline3D::Initialize(VkDevice device, const ShaderConfig& vertSConf, const ShaderConfig& fragSConf, const VkExtent2D& swapchainExtent, VkRenderPass renderPass)
{
	CreateDescriptorSetLayout(device);
	CreatePipelineLayout(device);
	CreatePipeline(device, vertSConf, fragSConf, swapchainExtent, renderPass);
}

void GraphicsPipeline3D::Draw(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet, uint32_t currentFrame) const
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	m_Scene.Draw(commandBuffer, m_VkPipelineLayout, currentFrame);
}

size_t GraphicsPipeline3D::GetNrOfModels() const
{
	return m_Scene.GetNrOfModels();
}

const VkDescriptorSetLayout& GraphicsPipeline3D::GetDescriptorSetLayout() const
{
	return m_VkDescriptorSetLayout;
}

void GraphicsPipeline3D::CreateDescriptorSetLayout(VkDevice device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings{ uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_VkDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void GraphicsPipeline3D::CreatePipelineLayout(VkDevice device)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(ModelUBO);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_VkDescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void GraphicsPipeline3D::CreatePipeline(VkDevice device, const ShaderConfig& vertSConf, const ShaderConfig& fragSConf, const VkExtent2D& swapchainExtent, VkRenderPass renderPass)
{
	Shader vertShader{};
	Shader fragShader{};
	vertShader.Initialize(device, vertSConf);
	fragShader.Initialize(device, fragSConf);

	auto vertShaderStage{ vertShader.GetPipelineShaderStageInfo() };
	auto fragShaderStage{ fragShader.GetPipelineShaderStageInfo() };
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{ vertShaderStage, fragShaderStage };

	auto vertexInputInfo{ Shader::GetVertex3DInputStateInfo() };
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
	// VK_POLYGON_MODE_FILL (fill the area of the polygon with fragments)
	// VK_POLYGON_MODE_LINE (polygon edges are drawn as lines)
	// VK_POLYGON_MODE_POINT (polygon vertices are drawn as points)
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

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

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VkPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vertShader.Destroy(device);
	fragShader.Destroy(device);
}

void GraphicsPipeline3D::InitScene(VkDevice device, VkPhysicalDevice phyDevice, VkQueue graphxQueue, const CommandPool& cmndPl)
{
	std::vector<Model3D> models{};

	Model3D model1{};
	model1.Initialize(device, phyDevice, graphxQueue, cmndPl, g_Model1Path);
	model1.SetPosition(glm::vec3{ -5.f, 0.5f, 0.f });

	Model3D model2{}; // plane
	model2.Initialize(device, phyDevice, graphxQueue, cmndPl, g_Model3Path);
	model2.SetScale(50.f);

	Model3D model3{}; // cube
	model3.Initialize(device, phyDevice, graphxQueue, cmndPl, g_Model2Path);
	model3.SetPosition(glm::vec3{ 2.f, 2.f, 2.f });
	model3.SetScale(0.5f);

	Model3D model4{}; // cube
	model4.Initialize(device, phyDevice, graphxQueue, cmndPl, g_Model2Path);
	model4.SetPosition(glm::vec3{ -2.f, 2.f, 2.f });
	model4.SetScale(0.5f);

	Model3D model5{}; // cube
	model5.Initialize(device, phyDevice, graphxQueue, cmndPl, g_Model2Path);
	model5.SetPosition(glm::vec3{ 2.f, 2.f, -2.f });
	model5.SetScale(0.5f);

	Model3D model6{}; // cube
	model6.Initialize(device, phyDevice, graphxQueue, cmndPl, g_Model2Path);
	model6.SetPosition(glm::vec3{ -2.f, 2.f, -2.f });
	model6.SetScale(0.5f);

	models.emplace_back(std::move(model1));
	models.emplace_back(std::move(model2));
	models.emplace_back(std::move(model3));
	models.emplace_back(std::move(model4));
	models.emplace_back(std::move(model5));
	models.emplace_back(std::move(model6));

	m_Scene.Initialize(std::move(models));
}

void GraphicsPipeline3D::Destroy(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, m_VkDescriptorSetLayout, nullptr);

	m_Scene.Destroy(device);

	vkDestroyPipeline(device, m_VkPipeline, nullptr);
	vkDestroyPipelineLayout(device, m_VkPipelineLayout, nullptr);
}