#include "Vulkan/PipelineLayout.h"
#include "Vulkan/DescriptorSetLayout.h"
#include "Vulkan/Device.h"

namespace vk {

	PipelineLayout::PipelineLayout(const Device& device, const DescriptorSetLayout& descriptorSetLayout) :
		device_(device)
	{
		VkDescriptorSetLayout descriptorSetLayouts[] = { descriptorSetLayout.Handle() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		Check(vkCreatePipelineLayout(device_.Handle(), &pipelineLayoutInfo, nullptr, &pipelineLayout_),
			"create pipeline layout");
	}

	PipelineLayout::PipelineLayout(const Device& device, const DescriptorSetLayout& descriptorSetLayout, const VkPushConstantRange& pushConstantRange) :
		device_(device)
	{
		VkDescriptorSetLayout descriptorSetLayouts[] = { descriptorSetLayout.Handle() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1; 
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; 

		Check(vkCreatePipelineLayout(device_.Handle(), &pipelineLayoutInfo, nullptr, &pipelineLayout_),
			"create pipeline layout");
	}

	PipelineLayout::~PipelineLayout()
	{
		if (pipelineLayout_ != nullptr)
		{
			vkDestroyPipelineLayout(device_.Handle(), pipelineLayout_, nullptr);
			pipelineLayout_ = nullptr;
		}
	}

}
