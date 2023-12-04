#pragma once

#include "Vulkan/WindowConfig.h"
#include "Vulkan/VkConfig.h"
#include <functional>
#include <vector>

namespace vk {
	class Window final {
	public:
		VULKAN_NON_COPIABLE(Window)

		explicit Window(const WindowConfig& config);
		~Window();

		// Window instance properties.
		const WindowConfig& Config() const { return config_; }
		GLFWwindow* Handle() const { return window_; }
		float ContentScale() const;
		VkExtent2D FramebufferSize() const;
		VkExtent2D WindowSize() const;

		// GLFW instance properties (i.e. not bound to a window handler).
		const char* GetKeyName(int key, int scancode) const;
		std::vector<const char*> GetRequiredInstanceExtensions() const;
		double GetTime() const;

		// Callbacks
		std::function<void()> DrawFrame;
		std::function<void(int key, int scancode, int action, int mods)> OnKey;
		std::function<void(double xpos, double ypos)> OnCursorPosition;
		std::function<void(int button, int action, int mods)> OnMouseButton;
		std::function<void(double xoffset, double yoffset)> OnScroll;

		// Methods
		void Close();
		bool IsMinimized() const;
		void Run();
		void WaitForEvents() const;
		void showFps();

	private:
		const WindowConfig config_;
		GLFWwindow* window_{};
	};
}