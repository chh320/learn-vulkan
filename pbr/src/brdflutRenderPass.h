#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/DepthBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"

class BrdfLutRenderPass final {
public:

	VULKAN_NON_COPIABLE(BrdfLutRenderPass)

	BrdfLutRenderPass(const vk::Device& device, VkFormat format);
	~BrdfLutRenderPass();

	const vk::Device& Device() const { return device_; }

private:

	const vk::Device& device_;

	VULKAN_HANDLE(VkRenderPass, renderPass_)
};