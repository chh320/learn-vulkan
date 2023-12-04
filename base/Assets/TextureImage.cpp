#include "Assets/TextureImage.h"
#include <cstring>

namespace Assets {

	TextureImage::TextureImage(vk::CommandPool& commandPool, const Texture& texture)
	{
		// Create a host staging buffer and copy the image into it.
		const VkDeviceSize imageSize = texture.Width() * texture.Height() * 4;
		const auto& device = commandPool.Device();

		auto stagingBuffer = std::make_unique<vk::Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		const auto data = stagingBufferMemory.Map(0, imageSize);
		std::memcpy(data, texture.Pixels(), imageSize);
		stagingBufferMemory.Unmap();

		// Create the device side image, memory, view and sampler.
		image_.reset(new vk::Image(device, VkExtent2D{ static_cast<uint32_t>(texture.Width()), static_cast<uint32_t>(texture.Height()) }, VK_FORMAT_R8G8B8A8_UNORM, 1, 1));
		imageMemory_.reset(new vk::DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new vk::ImageView(device, image_->Handle(), image_->Format(), VK_IMAGE_ASPECT_COLOR_BIT));
		sampler_.reset(new vk::Sampler(device, vk::SamplerConfig()));

		// Transfer the data to device side.
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		image_->CopyFrom(commandPool, *stagingBuffer);
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

		// Delete the buffer before the memory
		stagingBuffer.reset();
	}

	TextureImage::TextureImage(vk::CommandPool& commandPool, const GltfTexture& texture, const vk::SamplerConfig& sampler)
	{
		// Create a host staging buffer and copy the image into it.
		const VkDeviceSize imageSize = texture.Width() * texture.Height() * 4;
		const auto& device = commandPool.Device();

		auto stagingBuffer = std::make_unique<vk::Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		const auto data = stagingBufferMemory.Map(0, imageSize);
		std::memcpy(data, texture.Pixels(), imageSize);
		stagingBufferMemory.Unmap();

		// Create the device side image, memory, view and sampler.
		image_.reset(new vk::Image(device, VkExtent2D{ static_cast<uint32_t>(texture.Width()), static_cast<uint32_t>(texture.Height()) }, VK_FORMAT_R8G8B8A8_UNORM, 1, 1));
		imageMemory_.reset(new vk::DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new vk::ImageView(device, image_->Handle(), image_->Format(), VK_IMAGE_ASPECT_COLOR_BIT));
		sampler_.reset(new vk::Sampler(device, sampler));

		// Transfer the data to device side.
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 1);
		image_->CopyFrom(commandPool, *stagingBuffer);
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

		// Delete the buffer before the memory
		stagingBuffer.reset();
	}

	TextureImage::TextureImage(const vk::Device& device, const uint32_t dim, const VkFormat format, const VkImageUsageFlags usage)
	{
		// Create the device side image, memory, view and sampler.
		image_.reset(new vk::Image(device, VkExtent2D{ dim, dim }, format, VK_IMAGE_TILING_OPTIMAL, usage, 1, 1));
		imageMemory_.reset(new vk::DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new vk::ImageView(device, image_->Handle(), image_->Format(), VK_IMAGE_ASPECT_COLOR_BIT));
		sampler_.reset(new vk::Sampler(device, vk::SamplerConfig()));

	}

	TextureImage::~TextureImage()
	{
		sampler_.reset();
		imageView_.reset();
		image_.reset();
		imageMemory_.reset();
	}

	TextureCubeImage::TextureCubeImage(vk::CommandPool& commandPool, const uint32_t dim, const VkFormat format, const int32_t miplevels)
	{
		const auto& device = commandPool.Device();
		image_.reset(new vk::Image(device, VkExtent2D{ dim, dim }, format, miplevels, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT));
		imageMemory_.reset(new vk::DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new vk::CubeImageView(device, image_->Handle(), image_->Format(), VK_IMAGE_ASPECT_COLOR_BIT, miplevels));
		
		vk::SamplerConfig config{};
		config.MaxLod = static_cast<float>(miplevels);
		config.MaxAnisotropy = 1.f;
		config.BorderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler_.reset(new vk::Sampler(device, config));
;	}

	TextureCubeImage::~TextureCubeImage()
	{
		sampler_.reset();
		imageView_.reset();
		image_.reset();
		imageMemory_.reset();
	}
}
