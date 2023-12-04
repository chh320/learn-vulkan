#include "Assets/UniformBuffer.h"
#include <cstring>

namespace Assets {

	UniformBuffer::UniformBuffer(const vk::Device& device)
	{
		const auto bufferSize = sizeof(UniformBufferObject);

		buffer_.reset(new vk::Buffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT));
		memory_.reset(new vk::DeviceMemory(buffer_->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));
	}

	UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept :
		buffer_(other.buffer_.release()),
		memory_(other.memory_.release())
	{
	}

	UniformBuffer::~UniformBuffer()
	{
		buffer_.reset();
		memory_.reset(); // release memory after bound buffer has been destroyed
	}
}
