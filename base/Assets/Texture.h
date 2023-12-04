#pragma once

#include "Vulkan/Sampler.h"
#include "tiny_gltf.h"
#include <memory>
#include <string>

namespace Assets
{
	class Texture final
	{
	public:

		static Texture LoadTexture(const std::string& filename, const vk::SamplerConfig& samplerConfig);
		Texture& operator = (const Texture&) = delete;
		Texture& operator = (Texture&&) = delete;

		Texture() = default;
		Texture(const Texture&) = default;
		Texture(Texture&&) = default;
		Texture(int width, int height, int channels, unsigned char* pixels);
		~Texture() = default;

		const unsigned char* Pixels() const { return pixels_.get(); }
		int Width() const { return width_; }
		int Height() const { return height_; }

	private:
		vk::SamplerConfig samplerConfig_;
		int width_;
		int height_;
		int channels_;
		std::unique_ptr<unsigned char, void (*) (void*)> pixels_;
	};

	class GltfTexture final
	{
	public:

		static GltfTexture LoadTexture(tinygltf::Image& gltfimage);
		GltfTexture& operator = (const GltfTexture&) = delete;
		GltfTexture& operator = (GltfTexture&&) = delete;

		GltfTexture() = default;
		GltfTexture(const GltfTexture&) = default;
		GltfTexture(GltfTexture&&) = default;
		GltfTexture(int width, int height, int channels, unsigned char* pixels);
		~GltfTexture() = default;

		const unsigned char* Pixels() const { return pixels_; }
		int Width() const { return width_; }
		int Height() const { return height_; }

	private:
		int width_;
		int height_;
		int channels_;
		unsigned char* pixels_;
	};

}
