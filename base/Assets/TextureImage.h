#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/Device.h"
#include "Assets/Texture.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/CommandPool.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/Image.h"
#include "Vulkan/Sampler.h"
#include <memory>

namespace vk
{
	class CommandPool;
	class DeviceMemory;
	class Image;
	class ImageView;
	class CubeImageView;
	class Sampler;
	class SamplerConfig;
}

namespace Assets
{
	class Texture;

	class TextureImage final
	{
	public:

		TextureImage(const TextureImage&) = delete;
		TextureImage(TextureImage&&) = delete;
		TextureImage& operator = (const TextureImage&) = delete;
		TextureImage& operator = (TextureImage&&) = delete;

		TextureImage(vk::CommandPool& commandPool, const Texture& texture);
		TextureImage(vk::CommandPool& commandPool, const GltfTexture& texture, const vk::SamplerConfig& sampler);
		TextureImage(const vk::Device& device, const uint32_t dim, const VkFormat format, const VkImageUsageFlags usage);
		~TextureImage();

		const vk::ImageView& ImageView() const { return *imageView_; }
		const vk::Sampler& Sampler() const { return *sampler_; }

	private:

		std::unique_ptr<vk::Image> image_;
		std::unique_ptr<vk::DeviceMemory> imageMemory_;
		std::unique_ptr<vk::ImageView> imageView_;
		std::unique_ptr<vk::Sampler> sampler_;
	};

	class TextureCubeImage final
	{
	public:
		TextureCubeImage(const TextureCubeImage&) = delete;
		TextureCubeImage(TextureCubeImage&&) = delete;
		TextureCubeImage& operator = (const TextureCubeImage&) = delete;
		TextureCubeImage& operator = (TextureCubeImage&&) = delete;

		TextureCubeImage(vk::CommandPool& commandPool, const uint32_t dim, const VkFormat format, const int32_t miplevels);
		~TextureCubeImage();

		vk::Image& Image() { return *image_; }
		const vk::CubeImageView& ImageView() const { return *imageView_; }
		const vk::Sampler& Sampler() const { return *sampler_; }

	private:
		std::unique_ptr<vk::Image> image_;
		std::unique_ptr<vk::DeviceMemory> imageMemory_;
		std::unique_ptr<vk::CubeImageView> imageView_;
		std::unique_ptr<vk::Sampler> sampler_;
	};
}
