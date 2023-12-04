#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Vulkan [0, 1] depth range, instead of OpenGL [-1, +1]
#define GLM_FORCE_RIGHT_HANDED // Vulkan has a left handed coordinate system (same as DirectX), OpenGL is right handed
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_MESSAGES 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

#include "Vulkan/Buffer.h"

namespace vk
{
	class Buffer;
	class Device;
	class DeviceMemory;
}

namespace Assets
{
	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec3 cameraPos;
	};

	struct pbrValule {
		int32_t prefilteredCubeMipLevels;
		int32_t debugViewInputs = 0;
		int32_t debugViewEquation = 0;
	};

	class UniformBuffer
	{
	public:

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator = (const UniformBuffer&) = delete;
		UniformBuffer& operator = (UniformBuffer&&) = delete;

		explicit UniformBuffer(const vk::Device& device);
		UniformBuffer(UniformBuffer&& other) noexcept;

		template<typename T>
		UniformBuffer(const vk::Device& device, const T& ubo) {
			const auto bufferSize = sizeof(T);
			size = bufferSize;
			buffer_.reset(new vk::Buffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
			memory_.reset(new vk::DeviceMemory(buffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));
		}

		~UniformBuffer();

		const vk::Buffer& Buffer() const { return *buffer_; }
		const int32_t BufferSize() const { return size; }

		template<class T> void SetValue(const T& ubo) {
			const auto data = memory_->Map(0, sizeof(T));
			std::memcpy(data, &ubo, sizeof(ubo));
			memory_->Unmap();
		}

	private:
		int32_t size;
		std::unique_ptr<vk::Buffer> buffer_;
		std::unique_ptr<vk::DeviceMemory> memory_;
	};

}