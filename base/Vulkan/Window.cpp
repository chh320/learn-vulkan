#include "Vulkan/Window.h"
#include "../Utilities/StbImage.h"
#include <iostream>
#include <sstream>

namespace vk {
	namespace {
		void GlfwErrorCallback(const int error, const char* const description)
		{
			std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
		}

		void GlfwKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
		{
			auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (this_->OnKey)
			{
				this_->OnKey(key, scancode, action, mods);
			}
		}

		void GlfwCursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
		{
			auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (this_->OnCursorPosition)
			{
				this_->OnCursorPosition(xpos, ypos);
			}
		}

		void GlfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
		{
			auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (this_->OnMouseButton)
			{
				this_->OnMouseButton(button, action, mods);
			}
		}

		void GlfwScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
		{
			auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (this_->OnScroll)
			{
				this_->OnScroll(xoffset, yoffset);
			}
		}
	}

	Window::Window(const WindowConfig& config) : config_(config) {
		glfwSetErrorCallback(GlfwErrorCallback);
		if (!glfwInit()) {
			throw std::runtime_error("glfwInit() failed");
		}

		if (!glfwVulkanSupported()) {
			throw std::runtime_error("glfwVulkanSupported() failed");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, config.Resizable ? GLFW_TRUE : GLFW_FALSE);

		auto* const monitor = config.Fullscreen ? glfwGetPrimaryMonitor() : nullptr;

		window_ = glfwCreateWindow(config.Width, config.Height, config.Title.c_str(), monitor, nullptr);
		if (window_ == nullptr) {
			throw std::runtime_error("failed to create window.");
		}

		GLFWimage icon;
		icon.pixels = stbi_load("../textures/Vulkan.png", &icon.width, &icon.height, nullptr, 4);
		if (icon.pixels == nullptr) {
			throw std::runtime_error("failed to load icon.");
		}

		glfwSetWindowIcon(window_, 1, &icon);
		stbi_image_free(icon.pixels);

		if (config.CursorDisabled)
		{
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		glfwSetWindowUserPointer(window_, this);
		glfwSetKeyCallback(window_, GlfwKeyCallback);
		glfwSetCursorPosCallback(window_, GlfwCursorPositionCallback);
		glfwSetMouseButtonCallback(window_, GlfwMouseButtonCallback);
		glfwSetScrollCallback(window_, GlfwScrollCallback);
		glfwSetWindowPos(window_, 50, 100);
	}

	Window::~Window() {
		if (window_ != nullptr)
		{
			glfwDestroyWindow(window_);
			window_ = nullptr;
		}

		glfwTerminate();
		glfwSetErrorCallback(nullptr);
	}

	float Window::ContentScale() const
	{
		float xscale;
		float yscale;
		glfwGetWindowContentScale(window_, &xscale, &yscale);

		return xscale;
	}

	VkExtent2D Window::FramebufferSize() const
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	VkExtent2D Window::WindowSize() const
	{
		int width, height;
		glfwGetWindowSize(window_, &width, &height);
		return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}
	
	const char* Window::GetKeyName(const int key, const int scancode) const
	{
		return glfwGetKeyName(key, scancode);
	}

	std::vector<const char*> Window::GetRequiredInstanceExtensions() const
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
	}

	double Window::GetTime() const
	{
		return glfwGetTime();
	}

	void Window::Close()
	{
		glfwSetWindowShouldClose(window_, 1);
	}

	bool Window::IsMinimized() const
	{
		const auto size = FramebufferSize();
		return size.height == 0 && size.width == 0;
	}

	void Window::Run()
	{
		glfwSetTime(0.0);

		while (!glfwWindowShouldClose(window_))
		{
			glfwPollEvents();
			showFps();
			if (DrawFrame)
			{
				DrawFrame();
			}
		}
	}

	void Window::showFps(){
		static int nbFrames = 0;
		static float lastTime = 0.0f;
		// Measure speed
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		nbFrames++;
		if (delta >= 1.0) { // If last cout was more than 1 sec ago
			//cout << 1000.0 / double(nbFrames) << endl;

			double fps = double(nbFrames) / delta;

			std::stringstream ss;
			ss << "Vulkan Window " << " [" << fps << " FPS]";

			glfwSetWindowTitle(Handle(), ss.str().c_str());

			nbFrames = 0;
			lastTime = currentTime;
		}
	}

	void Window::WaitForEvents() const
	{
		glfwWaitEvents();
	}


}