#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/Device.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/CommandPool.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/DeviceMemory.h"
#include "Assets/Texture.h"
#include "Assets/TextureImage.h"
#include "Assets/UiPipeline.h"
//#include "imgui/imgui.h"
#include "imgui.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>


namespace Assets {
	class UserInterface final {
	public:
		UserInterface(
			vk::CommandPool& commandPool,
			const vk::RenderPass& renderPass
		);

		~UserInterface();

		UserInterface(const UserInterface&) = delete;
		UserInterface(UserInterface&&) = delete;
		UserInterface& operator = (const UserInterface&) = delete;
		UserInterface& operator = (UserInterface&&) = delete;

		void Draw(VkCommandBuffer cmdBuffer);

		template<typename T> bool checkbox(const char* caption, T* value) {
			bool val = (*value == 1);
			bool res = ImGui::Checkbox(caption, &val);
			*value = val;
			return res;
		}
		bool header(const char* caption);
		bool slider(const char* caption, float* value, float min, float max);
		bool combo(const char* caption, int32_t* itemindex, std::vector<std::string> items);
		bool combo(const char* caption, std::string& selectedkey, std::map<std::string, std::string> items);
		bool button(const char* caption);
		void text(const char* formatstr, ...);

		const Assets::TextureImage& FontTexture()const { return *fontTexture_; }
		Assets::UiPipeline& Pipeline() { return *pipeline_; }

		std::unique_ptr<vk::Buffer> vertexBuffer;
		std::unique_ptr<vk::DeviceMemory> vertexBufferMemory;
		float vertexBufferSize;

		std::unique_ptr<vk::Buffer> indexBuffer;
		std::unique_ptr<vk::DeviceMemory> indexBufferMemory;
		float indexBufferSize;

	private:
		std::unique_ptr<Assets::TextureImage> fontTexture_;
		std::unique_ptr<Assets::UiPipeline> pipeline_;
	};
}