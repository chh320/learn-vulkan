#pragma once

#include "Vulkan/VkConfig.h"

namespace vk {
	class Device;

	class ImageView final {
	public:
		VULKAN_NON_COPIABLE(ImageView)

		explicit ImageView(const Device& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int32_t layerCount = 1, int32_t baseLayer = 0, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
		~ImageView();

		const class Device& Device() const { return device_; }

	private:

		const class Device& device_;
		const VkImage image_;
		const VkFormat format_;

		VULKAN_HANDLE(VkImageView, imageView_)
	};

	class CubeImageView final {
	public:
		VULKAN_NON_COPIABLE(CubeImageView)

		explicit CubeImageView(const Device& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int32_t miplevels);
		~CubeImageView();

		const class Device& Device() const { return device_; }

	private:

		const class Device& device_;
		const VkImage image_;
		const VkFormat format_;

		VULKAN_HANDLE(VkImageView, imageView_)
	};
}