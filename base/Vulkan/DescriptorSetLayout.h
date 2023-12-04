#pragma once

#include "Vulkan/DescriptorBinding.h"
#include <vector>

namespace vk
{
	class Device;

	class DescriptorSetLayout final
	{
	public:

		VULKAN_NON_COPIABLE(DescriptorSetLayout)

		DescriptorSetLayout(const Device& device, const std::vector<DescriptorBinding>& descriptorBindings);
		~DescriptorSetLayout();

	private:

		const Device& device_;

		VULKAN_HANDLE(VkDescriptorSetLayout, layout_)
	};

}
