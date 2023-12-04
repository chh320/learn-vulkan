#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/DepthBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/DepthBuffer.h"

class DepthRenderPass final {
public:

	VULKAN_NON_COPIABLE(DepthRenderPass)

	DepthRenderPass(const vk::Device& device, const vk::DepthBuffer& depthBuffer);
	~DepthRenderPass();

	const vk::Device& Device() const { return device_; }
	const vk::DepthBuffer& DepthBuffer() const { return depthBuffer_; }

private:

	const vk::Device& device_;
	const vk::DepthBuffer& depthBuffer_;

	VULKAN_HANDLE(VkRenderPass, renderPass_)
};