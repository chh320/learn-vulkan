#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/DepthBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"

class CubeMapRenderPass final {
public:

	VULKAN_NON_COPIABLE(CubeMapRenderPass)

	CubeMapRenderPass(const vk::Device& device, VkFormat format);
	~CubeMapRenderPass();

	const vk::Device& Device() const { return device_; }

private:

	const vk::Device& device_;

	VULKAN_HANDLE(VkRenderPass, renderPass_)
};