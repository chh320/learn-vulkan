#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/DeviceMemory.h"

namespace vk {
	class Buffer;
	class CommandPool;
	class Device;

	class Image final {
	public:
		Image(const Image&) = delete;
		Image& operator = (const Image&) = delete;
		Image& operator = (Image&&) = delete;

		Image(const Device& device, VkExtent2D extent, VkFormat format, const int32_t miplevels, const int32_t arrayLayers, const int32_t flag = 0);
		Image(const Device& device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, const int32_t miplevels, const int32_t arrayLayers, const int32_t flag = 0);
		Image(Image&& other) noexcept;
		~Image();

		const class Device& Device() const { return device_; }
		VkExtent2D Extent() const { return extent_; }
		VkFormat Format() const { return format_; }
		VkImageLayout ImageLayout() const { return imageLayout_; }

		DeviceMemory AllocateMemory(VkMemoryPropertyFlags properties) const;
		VkMemoryRequirements GetMemoryRequirements() const;

		void TransitionImageLayout(CommandPool& commandPool, VkImageLayout newLayout, const int32_t levelCount, const int32_t layerCount);
		void CopyFrom(CommandPool& commandPool, const Buffer& buffer);

	private:
		const class Device& device_;
		const VkExtent2D extent_;
		const VkFormat format_;
		VkImageLayout imageLayout_;

		VULKAN_HANDLE(VkImage, image_)
	};
}