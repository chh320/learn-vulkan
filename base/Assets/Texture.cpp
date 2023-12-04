#include "Assets/Texture.h"
#include "Utilities/StbImage.h"
#include <stdexcept>
#include <chrono>
#include <iostream>

namespace Assets {

	Texture Texture::LoadTexture(const std::string& filename, const vk::SamplerConfig& samplerConfig)
	{
		std::cout << "- loading '" << filename << "'... " << std::flush;
		const auto timer = std::chrono::high_resolution_clock::now();

		// Load the texture in normal host memory.
		int width, height, channels;
		const auto pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image '" + filename + "'");
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();
		std::cout << "(" << width << " x " << height << " x " << channels << ") ";
		std::cout << elapsed << "s" << std::endl;

		return Texture(width, height, channels, pixels);
	}

	Texture::Texture(int width, int height, int channels, unsigned char* const pixels) :
		width_(width),
		height_(height),
		channels_(channels),
		pixels_(pixels, stbi_image_free)
	{
	}

	GltfTexture GltfTexture::LoadTexture(tinygltf::Image& gltfimage) {
		unsigned char* buffer;
		VkDeviceSize bufferSize = gltfimage.width * gltfimage.height * 4;
		buffer = new unsigned char[bufferSize];
		unsigned char* rgba = buffer;
		unsigned char* rgb = &gltfimage.image[0];

		int32_t channel = gltfimage.component;
		for (int32_t i = 0; i < gltfimage.width * gltfimage.height; ++i) {
			for (int32_t j = 0; j < channel; ++j) {
				rgba[j] = rgb[j];
			}
			rgba += 4;
			rgb += channel;
		}
		int width = gltfimage.width;
		int height = gltfimage.height;
		int channels = 4;
		return GltfTexture(width, height, channels, buffer);
	}

	GltfTexture::GltfTexture(int width, int height, int channels, unsigned char* const pixels) :
		width_(width),
		height_(height),
		channels_(channels),
		pixels_(pixels)
	{
	}
}
