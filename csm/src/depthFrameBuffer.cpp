#include "depthFrameBuffer.h"

#include <array>

DepthFrameBuffer::DepthFrameBuffer(int id, const DepthRenderPass& renderPass, const int32_t dim) :
	renderPass_(renderPass)
{
	imageView_.reset(new vk::ImageView(renderPass.Device(), renderPass.DepthBuffer().Image().Handle(), renderPass.DepthBuffer().Format(), 
		VK_IMAGE_ASPECT_DEPTH_BIT, 1, id, VK_IMAGE_VIEW_TYPE_2D_ARRAY));

	std::array<VkImageView, 1> attachments =
	{
		imageView_->Handle()
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass.Handle();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = dim;
	framebufferInfo.height = dim;
	framebufferInfo.layers = 1;

	vk::Check(vkCreateFramebuffer(imageView_->Device().Handle(), &framebufferInfo, nullptr, &framebuffer_),
		"create framebuffer");
}

DepthFrameBuffer::DepthFrameBuffer(DepthFrameBuffer&& other) noexcept :
	imageView_(std::move(other.imageView_)),
	renderPass_(other.renderPass_),
	framebuffer_(other.framebuffer_)
{
	other.framebuffer_ = nullptr;
}

DepthFrameBuffer::~DepthFrameBuffer()
{
	if (framebuffer_ != nullptr)
	{
		vkDestroyFramebuffer(imageView_->Device().Handle(), framebuffer_, nullptr);
		framebuffer_ = nullptr;
	}
}


