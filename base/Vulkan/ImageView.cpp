#include "Vulkan/ImageView.h"
#include "Vulkan/Device.h"

namespace vk {
	ImageView::ImageView(const class Device& device, const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, int32_t layerCount, int32_t baseLayer, VkImageViewType viewType) :
		device_(device),
		image_(image),
		format_(format)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = viewType;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = baseLayer;
		createInfo.subresourceRange.layerCount = layerCount;

		Check(vkCreateImageView(device_.Handle(), &createInfo, nullptr, &imageView_),
			"create image view");
	}

	ImageView::~ImageView()
	{
		if (imageView_ != nullptr)
		{
			vkDestroyImageView(device_.Handle(), imageView_, nullptr);
			imageView_ = nullptr;
		}
	}

	CubeImageView::CubeImageView(const vk::Device& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int32_t miplevels) :
		device_(device),
		image_(image),
		format_(format) 
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		createInfo.format = format;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = miplevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 6;

		Check(vkCreateImageView(device_.Handle(), &createInfo, nullptr, &imageView_),
			"create image view");
	}

	CubeImageView::~CubeImageView() {
		if (imageView_ != nullptr)
		{
			vkDestroyImageView(device_.Handle(), imageView_, nullptr);
			imageView_ = nullptr;
		}
	}
}