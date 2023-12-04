#include "cubemapFrameBuffer.h"

#include <array>

CubemapFrameBuffer::CubemapFrameBuffer(const vk::ImageView& imageView, const CubeMapRenderPass& renderPass, const int32_t dim) :
	imageView_(imageView),
	renderPass_(renderPass)
{
	std::array<VkImageView, 1> attachments =
	{
		imageView.Handle(),
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass.Handle();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = dim;
	framebufferInfo.height = dim;
	framebufferInfo.layers = 1;

	vk::Check(vkCreateFramebuffer(imageView_.Device().Handle(), &framebufferInfo, nullptr, &framebuffer_),
		"create framebuffer");
}

CubemapFrameBuffer::CubemapFrameBuffer(CubemapFrameBuffer&& other) noexcept :
	imageView_(other.imageView_),
	renderPass_(other.renderPass_),
	framebuffer_(other.framebuffer_)
{
	other.framebuffer_ = nullptr;
}

CubemapFrameBuffer::~CubemapFrameBuffer()
{
	if (framebuffer_ != nullptr)
	{
		vkDestroyFramebuffer(imageView_.Device().Handle(), framebuffer_, nullptr);
		framebuffer_ = nullptr;
	}
}


