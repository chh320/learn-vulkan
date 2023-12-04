#include "Vulkan/DescriptorSetManager.h"
#include "Vulkan/DescriptorPool.h"
#include "Vulkan/DescriptorSetLayout.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Device.h"
#include <stdexcept>
#include <set>

namespace vk {

	DescriptorSetManager::DescriptorSetManager(const Device& device, const std::vector<DescriptorBinding>& descriptorBindings, const size_t maxSets)
	{
		// Sanity check to avoid binding different resources to the same binding point.
		std::map<uint32_t, VkDescriptorType> bindingTypes;

		for (const auto& binding : descriptorBindings)
		{
			if (!bindingTypes.insert(std::make_pair(binding.Binding, binding.Type)).second)
			{
				throw std::invalid_argument("binding collision");
			}
		}

		descriptorPool_.reset(new DescriptorPool(device, descriptorBindings, maxSets));
		descriptorSetLayout_.reset(new class DescriptorSetLayout(device, descriptorBindings));
		descriptorSets_.reset(new class DescriptorSets(*descriptorPool_, *descriptorSetLayout_, bindingTypes, maxSets));
	}

	DescriptorSetManager::~DescriptorSetManager()
	{
		descriptorSets_.reset();
		descriptorSetLayout_.reset();
		descriptorPool_.reset();
	}

}
