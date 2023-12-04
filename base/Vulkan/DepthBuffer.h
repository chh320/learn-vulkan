#pragma once

#include "Vulkan/VkConfig.h"
#include <memory>

namespace vk {
	class CommandPool;
	class Device;
	class DeviceMemory;
	class Image;
	class ImageView;
	class Sampler;

	class DepthBuffer final {
	public:
		VULKAN_NON_COPIABLE(DepthBuffer)

		DepthBuffer(CommandPool& commandPool, const VkExtent2D extent, const int32_t arrayLayers);
		DepthBuffer(CommandPool& commandPool, VkExtent2D extent, const int32_t arrayLayers, bool shadowMap);
		~DepthBuffer();

		VkFormat Format() const { return format_; }
		const class ImageView& ImageView() const { return *imageView_; }
		const class Image& Image() const { return *image_; }
		const class Sampler& Sampler() const { return *sampler_; }

		static bool HasStencilComponent(const VkFormat format) {
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

	private:
		const VkFormat format_;
		std::unique_ptr<class Image> image_;
		std::unique_ptr<DeviceMemory> imageMemory_;
		std::unique_ptr<class ImageView> imageView_;
		std::unique_ptr<vk::Sampler> sampler_;
	};
}