#pragma once

#include "Vulkan/VkConfig.h"
#include <shaderc/shaderc.hpp>
#include <string>
#include <vector>

namespace vk
{
	class Device;

	class ShaderModule final
	{
	public:

		VULKAN_NON_COPIABLE(ShaderModule)

		ShaderModule(const Device& device, const std::string& filename);
		ShaderModule(const Device& device, const std::vector<char>& code);
		ShaderModule(const Device& device, const std::vector<uint32_t>& code);
		~ShaderModule();

		const class Device& Device() const { return device_; }

		VkPipelineShaderStageCreateInfo CreateShaderStage(VkShaderStageFlagBits stage) const;
		static std::vector<uint32_t> ReadFile(const std::string& filename, shaderc_shader_kind kind, bool optimize = false);

	private:

		static std::vector<char> ReadFile(const std::string& filename);

		const class Device& device_;

		VULKAN_HANDLE(VkShaderModule, shaderModule_)
	};

}
