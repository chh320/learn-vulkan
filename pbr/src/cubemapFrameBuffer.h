#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/DepthBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/SwapChain.h"
#include "cubemapRenderPass.h"

class CubemapFrameBuffer final {
public:
	CubemapFrameBuffer(const CubemapFrameBuffer&) = delete;
	CubemapFrameBuffer& operator = (const CubemapFrameBuffer&) = delete;
	CubemapFrameBuffer& operator = (CubemapFrameBuffer&&) = delete;

	explicit CubemapFrameBuffer(const vk::ImageView& imageView, const CubeMapRenderPass& renderPass, const int32_t dim);
	CubemapFrameBuffer(CubemapFrameBuffer&& other) noexcept;
	~CubemapFrameBuffer();

	const vk::ImageView& ImageView() const { return imageView_; }
	const CubeMapRenderPass& RenderPass() const { return renderPass_; }

private:
	const vk::ImageView& imageView_;
	const CubeMapRenderPass& renderPass_;

	VULKAN_HANDLE(VkFramebuffer, framebuffer_);
};