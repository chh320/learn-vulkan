#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/DepthBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Image.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/SwapChain.h"
#include "depthRenderPass.h"

class DepthFrameBuffer final {
public:
	DepthFrameBuffer(const DepthFrameBuffer&) = delete;
	DepthFrameBuffer& operator = (const DepthFrameBuffer&) = delete;
	DepthFrameBuffer& operator = (DepthFrameBuffer&&) = delete;

	explicit DepthFrameBuffer(int id, const DepthRenderPass& renderPass, const int32_t dim);
	DepthFrameBuffer(DepthFrameBuffer&& other) noexcept;
	~DepthFrameBuffer();

	const vk::ImageView& ImageView() const { return *imageView_; }
	const DepthRenderPass& RenderPass() const { return renderPass_; }

private:
	std::unique_ptr< vk::ImageView> imageView_;
	const DepthRenderPass& renderPass_;

	VULKAN_HANDLE(VkFramebuffer, framebuffer_);
};