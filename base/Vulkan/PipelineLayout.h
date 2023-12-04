#pragma once

#include "Vulkan/VkConfig.h"

namespace vk
{
	class DescriptorSetLayout;
	class Device;

	class PipelineLayout final
	{
	public:

		VULKAN_NON_COPIABLE(PipelineLayout)

		PipelineLayout(const Device& device, const DescriptorSetLayout& descriptorSetLayout);
		PipelineLayout(const Device& device, const DescriptorSetLayout& descriptorSetLayout, const VkPushConstantRange& pushConstantRange);
		~PipelineLayout();

	private:

		const Device& device_;

		VULKAN_HANDLE(VkPipelineLayout, pipelineLayout_)
	};

}
