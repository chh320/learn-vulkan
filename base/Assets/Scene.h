#pragma once

#include "Vulkan/VkConfig.h"
#include <memory>
#include <vector>

namespace vk
{
	class Buffer;
	class CommandPool;
	class DeviceMemory;
	class Image;
}

namespace Assets
{
	class Model;
	class GltfModel;
	class Texture;
	class TextureImage;

	class Scene final
	{
	public:

		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator = (const Scene&) = delete;
		Scene& operator = (Scene&&) = delete;

		Scene(vk::CommandPool& commandPool, std::vector<Model>&& models, std::vector<Texture>&& textures);
		Scene(vk::CommandPool& commandPool, std::vector<GltfModel>&& models);
		~Scene();

		const std::vector<Model>& Models() const { return models_; }
		const vk::Buffer& VertexBuffer() const { return *vertexBuffer_; }
		const vk::Buffer& IndexBuffer() const { return *indexBuffer_; }
		const std::vector<VkImageView> TextureImageViews() const { return textureImageViewHandles_; }
		const std::vector<VkSampler> TextureSamplers() const { return textureSamplerHandles_; }

	private:

		const std::vector<Model> models_;
		const std::vector<Texture> textures_;

		std::unique_ptr<vk::Buffer> vertexBuffer_;
		std::unique_ptr<vk::DeviceMemory> vertexBufferMemory_;

		std::unique_ptr<vk::Buffer> indexBuffer_;
		std::unique_ptr<vk::DeviceMemory> indexBufferMemory_;

		std::vector<std::unique_ptr<TextureImage>> textureImages_;
		std::vector<VkImageView> textureImageViewHandles_;
		std::vector<VkSampler> textureSamplerHandles_;
	};

}
