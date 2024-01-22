#include "Vulkan/DepthBuffer.h"
#include "Vulkan/CommandPool.h"
#include "Vulkan/Device.h"
#include "Vulkan/DeviceMemory.h"
#include "Vulkan/Image.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/Sampler.h"
#include <stdexcept>

namespace vk {
	namespace {
		VkFormat FindSupportedFormat(const Device& device, const std::vector<VkFormat>& candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features)
		{
			for (auto format : candidates)
			{
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(device.PhysicalDevice(), format, &props);

				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				{
					return format;
				}

				if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				{
					return format;
				}
			}

			throw std::runtime_error("failed to find supported format");
		}

		VkFormat FindDepthFormat(const Device& device)
		{
			return FindSupportedFormat(
				device,
				{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
		}
	}

	DepthBuffer::DepthBuffer(CommandPool& commandPool, const VkExtent2D extent, const int32_t arrayLayers) :
		format_(FindDepthFormat(commandPool.Device()))
	{
		const auto& device = commandPool.Device();

		image_.reset(new class Image(device, extent, format_, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, arrayLayers));
		imageMemory_.reset(new DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new class ImageView(device, image_->Handle(), format_, VK_IMAGE_ASPECT_DEPTH_BIT, arrayLayers, 0, arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D));

		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, arrayLayers);

		const auto& debugUtils = device.DebugUtils();

		debugUtils.SetObjectName(image_->Handle(), "Depth Buffer Image");
		debugUtils.SetObjectName(imageMemory_->Handle(), "Depth Buffer Image Memory");
		debugUtils.SetObjectName(imageView_->Handle(), "Depth Buffer ImageView");
	}

	DepthBuffer::DepthBuffer(CommandPool& commandPool, const VkExtent2D extent, const int32_t arrayLayers, bool shadowMap) :
		format_(FindDepthFormat(commandPool.Device()))
	{
		const auto& device = commandPool.Device();

		image_.reset(new class Image(device, extent, format_, VK_IMAGE_TILING_OPTIMAL, 
			shadowMap ? (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) 
			: (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
			1, arrayLayers));
		imageMemory_.reset(new DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new class ImageView(device, image_->Handle(), format_, VK_IMAGE_ASPECT_DEPTH_BIT, arrayLayers, 0, arrayLayers > 1? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D));

		if (shadowMap) {
			sampler_.reset(new vk::Sampler(device, vk::SamplerConfig()));
		}

		//image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, arrayLayers);

		const auto& debugUtils = device.DebugUtils();

		debugUtils.SetObjectName(image_->Handle(), "Depth Buffer Image");
		debugUtils.SetObjectName(imageMemory_->Handle(), "Depth Buffer Image Memory");
		debugUtils.SetObjectName(imageView_->Handle(), "Depth Buffer ImageView");
	}

	DepthBuffer::~DepthBuffer()
	{
		imageView_.reset();
		image_.reset();
		imageMemory_.reset(); // release memory after bound image has been destroyed
	}
}