#pragma once
#include "Vulkan/Application.h"
#include "Vulkan/DepthBuffer.h"
#include "Assets/Camera.h"
#include "Assets/TextureImage.h"
#include "Assets/UserInterface.h"
#include "Assets/UniformBuffer.h"
#include "Assets/GltfModel.h"
#include "scenePipeline.h"
#include "depthPipeline.h"
#include "depthFrameBuffer.h"
#include <map>
#include <string>

class Renderer final: public vk::Application
{
public:

	VULKAN_NON_COPIABLE(Renderer);

	Renderer(const vk::WindowConfig& windowConfig, VkPresentModeKHR presentMode, bool enableValidationLayers);
	~Renderer();

protected:
	void OnDeviceSet() override;

	const Assets::Scene& GetScene() const override { return *scene_; }
	Assets::UniformBufferObject GetUniformBufferObject(VkExtent2D extent) const override;
	Assets::UserInterface& UI() const { return *ui_; }
	vk::DepthBuffer& GetCascadedDepthBuffer() const { return *cascadedDepthBuffer_; }
	const Assets::UniformBuffer& GetLightUniformBuffer() const { return *lightUniformBuffer_; }
	const Assets::UniformBuffer& GetShadowUniformBuffer() const { return *shadowUniformBuffer_; }

	void CreateSwapChain() override;
	void DeleteSwapChain() override;
	void Render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
	void OnKey(int key, int scancode, int action, int mods) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnCursorPosition(double xpos, double ypos) override;
	void OnMouseButton(int button, int action, int mods) override;

	void Prepare();
	void UpdateUi();
	void UpdateLight();
	void UpdateCascades();
	void RenderScene();

	const bool& GetMouseLeftDown() const { return mouseStatus_.lDown; }
	const bool& GetMouseRightDown() const { return mouseStatus_.rDown; }
	const int32_t& GetMouseHorizontalMove() const { return mouseStatus_.horizontalMove; }
	const int32_t& GetMouseVerticalMove() const { return mouseStatus_.verticalMove; }
	void CleanUpMouseStatus() {
		mouseStatus_.verticalMove = 0;
		mouseStatus_.horizontalMove = 0;
	}

private:
	void LoadScene();

	std::unique_ptr<const Assets::Scene> scene_;
	std::unique_ptr<Assets::Camera> camera_;
	std::unique_ptr<class Assets::UserInterface> ui_;
	std::unique_ptr<ScenePipeline> scenePipeline_;
	std::unique_ptr<DepthPipeline> depthPipeline_;
	std::vector<DepthFrameBuffer> depthFrameBuffer_;
	std::unique_ptr<vk::DepthBuffer> cascadedDepthBuffer_;
	std::unique_ptr<Assets::UniformBuffer> lightUniformBuffer_;
	std::unique_ptr<Assets::UniformBuffer> shadowUniformBuffer_;

	struct {
		bool lDown = false, rDown = false;
		int32_t horizontalMove = 0, verticalMove = 0;
		int32_t lastX, lastY;
	} mouseStatus_;

	struct LightPara {
		std::array<glm::mat4, SHADOW_MAP_CASCADE_COUNT> viewProjMatrix;
	} lightUBO_;

	struct ShadowUBO {
		glm::vec4 splitDepth;
		glm::mat4 viewProjMatrix[SHADOW_MAP_CASCADE_COUNT];
		glm::vec4 splitSphereBound[SHADOW_MAP_CASCADE_COUNT];
		glm::vec3 lightDir;
	} shadowUBO_;

	const float cascadeSplitLambda = 0.95f;
	glm::vec3 lightPos;
	bool colorCascades;
};
