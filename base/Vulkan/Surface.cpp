#include "Vulkan/Surface.h"
#include "Vulkan/Instance.h"
#include "Vulkan/Window.h"

namespace vk {
	Surface::Surface(const class Instance& instance) : instance_(instance) {
		Check(glfwCreateWindowSurface(instance.Handle(), instance.Window().Handle(), nullptr, &surface_),
			"create window surface");
	}

	Surface::~Surface() {
		if (surface_ != nullptr)
		{
			vkDestroySurfaceKHR(instance_.Handle(), surface_, nullptr);
			surface_ = nullptr;
		}
	}
}