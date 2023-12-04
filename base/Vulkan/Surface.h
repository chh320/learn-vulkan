#pragma once

#include "Vulkan/VkConfig.h"

namespace vk {
	class Instance;
	class Window;

	class Surface final {
	public:
		VULKAN_NON_COPIABLE(Surface)

		explicit Surface(const Instance& instace);
		~Surface();

		const class Instance& Instance() const { return instance_; }

	private:
		const class Instance& instance_;

		VULKAN_HANDLE(VkSurfaceKHR, surface_)
	};
}