#pragma once
#include "Vulkan/Application.h"
#include "Assets/Camera.h"
#include "Assets/TextureImage.h"
#include "Assets/UserInterface.h"
#include "Assets/UniformBuffer.h"
#include "Assets/GltfModel.h"
#include "skyboxPipeline.h"
#include "pbrPipeline.h"
#include "cubemapPipeline.h"
#include "cubemapFrameBuffer.h"
#include "brdflutPipeline.h"
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
	const Assets::Scene& GetSkybox() const { return *skybox_; }
	const Assets::TextureCubeImage& GetIrradianceMap() const { return *irradianceMap_; }
	const Assets::TextureCubeImage& GetPrefilteredMap() const { return *prefilterMap_; }
	const Assets::TextureImage& GetBrdfLutMap() const { return *brdfLut_; }
	const Assets::UniformBuffer& GetShaderValuesUBO() const { return *shaderValuesUBO_; }
	Assets::UniformBufferObject GetUniformBufferObject(VkExtent2D extent) const override;
	Assets::UserInterface& UI() const { return *ui_; }

	void CreateSwapChain() override;
	void DeleteSwapChain() override;
	void Render(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;
	void OnKey(int key, int scancode, int action, int mods) override;
	void OnScroll(double xoffset, double yoffset) override;
	void OnCursorPosition(double xpos, double ypos) override;
	void OnMouseButton(int button, int action, int mods) override;

	void Prepare();
	void GenerateCubemaps();
	void GenratateBRDFLUT();
	void UpdateUi();
	void UpdateUBO();

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
	std::unique_ptr<const Assets::Scene> skybox_;
	std::unique_ptr<const SkyBoxPipeline> skyboxPipeline_;
	std::unique_ptr<const PbrPipeline> pbrPipeline_;
	std::unique_ptr<CubeMapPipeline> cubemapPipeline_;
	std::unique_ptr<BrdfLutPipeline> brdflutPipeline_;
	std::unique_ptr<Assets::Camera> camera_;
	std::unique_ptr<Assets::TextureCubeImage> irradianceMap_;
	std::unique_ptr<Assets::TextureCubeImage> prefilterMap_;
	std::unique_ptr<Assets::TextureImage> brdfLut_;
	std::unique_ptr<Assets::UniformBuffer> shaderValuesUBO_;
	std::unique_ptr<class Assets::UserInterface> ui_;

	struct {
		bool lDown = false, rDown = false;
		int32_t horizontalMove = 0, verticalMove = 0;
		int32_t lastX, lastY;
	} mouseStatus_;

	int32_t debugViewInputs = 0;
	int32_t debugViewEquation = 0;

	Assets::pbrValule shaderValuesParams_;
};
