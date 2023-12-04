#include "brdflutPipeline.h"
#include <iostream>

BrdfLutPipeline::BrdfLutPipeline(
	const vk::Device& device,
	VkFormat format)
	: device_(device)
{
	const auto bindingDescription = Assets::Vertex::GetBindingDescription();
	const auto attributeDescriptions = std::vector<VkVertexInputAttributeDescription>{ {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0} };

	// No vertex need.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
	dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Create descriptor pool/sets.
	std::vector<vk::DescriptorBinding> descriptorBindings ={};
	descriptorSetManager_.reset(new vk::DescriptorSetManager(device, descriptorBindings, 1));

	auto& descriptorSets = descriptorSetManager_->DescriptorSets();

	const std::vector<VkWriteDescriptorSet> descriptorWrites = {};
	descriptorSets.UpdateDescriptors(0, descriptorWrites);

	// Create pipeline layout and render pass.
	pipelineLayout_.reset(new class vk::PipelineLayout(device, descriptorSetManager_->DescriptorSetLayout()));
	renderPass_.reset(new BrdfLutRenderPass(Device(), format));

	// Load shaders.
	auto vert_code = vk::ShaderModule::ReadFile("../shaders/genbrdflut.vert", shaderc_glsl_vertex_shader);
	auto frag_code = vk::ShaderModule::ReadFile("../shaders/genbrdflut.frag", shaderc_glsl_fragment_shader);

	const vk::ShaderModule vertShader(device, vert_code);
	const vk::ShaderModule fragShader(device, frag_code);

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShader.CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT),
		fragShader.CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	// Create graphic pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateCI;
	pipelineInfo.layout = pipelineLayout_->Handle();
	pipelineInfo.renderPass = renderPass_->Handle();
	pipelineInfo.subpass = 0;

	vk::Check(vkCreateGraphicsPipelines(device.Handle(), nullptr, 1, &pipelineInfo, nullptr, &pipeline_),
		"create graphics pipeline");
}

BrdfLutPipeline::~BrdfLutPipeline()
{
	if (pipeline_ != nullptr) {
		vkDestroyPipeline(Device().Handle(), pipeline_, nullptr);
		pipeline_ = nullptr;
	}

	pipelineLayout_.reset();
	descriptorSetManager_.reset();
}

VkDescriptorSet BrdfLutPipeline::DescriptorSet(uint32_t index) const
{
	return descriptorSetManager_->DescriptorSets().Handle(index);
}