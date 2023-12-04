#include "Assets/Scene.h"
#include "Assets/Model.h"
#include "Assets/GltfModel.h"
#include "Assets/Texture.h"
#include "Assets/TextureImage.h"
#include "Vulkan/BufferUtil.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/Sampler.h"
#include "Vulkan/SingleTimeCommands.h"
#include <stdexcept>



namespace Assets {

	Scene::Scene(vk::CommandPool& commandPool, std::vector<Model>&& models, std::vector<Texture>&& textures) :
		models_(std::move(models)),
		textures_(std::move(textures))
	{
		// Concatenate all the models
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<glm::uvec2> offsets;

		for (const auto& model : models_)
		{
			// Remember the index, vertex offsets.
			const auto indexOffset = static_cast<uint32_t>(indices.size());
			const auto vertexOffset = static_cast<uint32_t>(vertices.size());

			offsets.emplace_back(indexOffset, vertexOffset);

			// Copy model data one after the other.
			vertices.insert(vertices.end(), model.Vertices().begin(), model.Vertices().end());
			indices.insert(indices.end(), model.Indices().begin(), model.Indices().end());

		}

		//constexpr auto flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		vk::BufferUtil::CreateDeviceBuffer(commandPool, "Vertices", VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR , vertices, vertexBuffer_, vertexBufferMemory_);
		vk::BufferUtil::CreateDeviceBuffer(commandPool, "Indices", VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR , indices, indexBuffer_, indexBufferMemory_);

		// Upload all textures
		textureImages_.reserve(textures_.size());
		textureImageViewHandles_.resize(textures_.size());
		textureSamplerHandles_.resize(textures_.size());

		for (size_t i = 0; i != textures_.size(); ++i)
		{
			textureImages_.emplace_back(new TextureImage(commandPool, textures_[i]));
			textureImageViewHandles_[i] = textureImages_[i]->ImageView().Handle();
			textureSamplerHandles_[i] = textureImages_[i]->Sampler().Handle();
		}
	}

	Scene::Scene(vk::CommandPool& commandPool, std::vector<GltfModel>&& models)
	{
		// Concatenate all the models
		std::vector<GltfVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<glm::uvec2> offsets;

		for (const auto& model : models)
		{
			// Remember the index, vertex offsets.
			const auto indexOffset = static_cast<uint32_t>(indices.size());
			const auto vertexOffset = static_cast<uint32_t>(vertices.size());

			offsets.emplace_back(indexOffset, vertexOffset);

			// Copy model data one after the other.
			vertices.insert(vertices.end(), model.Vertices().begin(), model.Vertices().end());
			indices.insert(indices.end(), model.Indices().begin(), model.Indices().end());

			// Upload all textures
			textureImages_.reserve(model.Textures().size());
			textureImageViewHandles_.resize(model.Textures().size());
			textureSamplerHandles_.resize(model.Textures().size());

			for (size_t i = 0; i != model.Textures().size(); ++i)
			{
				textureImages_.emplace_back(new TextureImage(commandPool, model.Textures()[i], model.Sampler()[0]));
				textureImageViewHandles_[i] = textureImages_[i]->ImageView().Handle();
				textureSamplerHandles_[i] = textureImages_[i]->Sampler().Handle();
			}
		}

		//constexpr auto flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		vk::BufferUtil::CreateDeviceBuffer(commandPool, "Vertices", VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, vertices, vertexBuffer_, vertexBufferMemory_);
		vk::BufferUtil::CreateDeviceBuffer(commandPool, "Indices", VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, indices, indexBuffer_, indexBufferMemory_);


	}

	Scene::~Scene()
	{
		textureSamplerHandles_.clear();
		textureImageViewHandles_.clear();
		textureImages_.clear();
		indexBuffer_.reset();
		indexBufferMemory_.reset(); // release memory after bound buffer has been destroyed
		vertexBuffer_.reset();
		vertexBufferMemory_.reset(); // release memory after bound buffer has been destroyed
	}

}
