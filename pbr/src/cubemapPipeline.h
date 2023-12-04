#pragma once

#include "Vulkan/VkConfig.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/DescriptorSetManager.h"
#include "Vulkan/DescriptorPool.h"
#include "Vulkan/DescriptorSets.h"
#include "Vulkan/Device.h"
#include "Vulkan/PipelineLayout.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/ShaderModule.h"
#include "Vulkan/SwapChain.h"
#include "Assets/Scene.h"
#include "Assets/Vertex.h"
#include "Assets/UniformBuffer.h"
#include "cubemapRenderPass.h"
#include <memory>
#include <vector>

#define M_PI 3.14159265358979323846

class CubeMapPipeline final
{
public:

	struct PushBlockIrradiance {
		glm::mat4 mvp;
		float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
		float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
	} pushBlockIrradiance;

	struct PushBlockPrefilterEnv {
		glm::mat4 mvp;
		float roughness;
		uint32_t numSamples = 32u;
	} pushBlockPrefilterEnv;

	VULKAN_NON_COPIABLE(CubeMapPipeline)

	CubeMapPipeline(
		const vk::Device& device,
		const Assets::Scene& scene,
		VkFormat format,
		const uint32_t target);
	~CubeMapPipeline();

	VkDescriptorSet DescriptorSet(uint32_t index) const;
	const vk::PipelineLayout& PipelineLayout() const { return *pipelineLayout_; }
	const CubeMapRenderPass& RenderPass() const { return *renderPass_; }
	const vk::Device& Device() const { return device_; }

private:

	VULKAN_HANDLE(VkPipeline, pipeline_)

	const vk::Device& device_;

	std::unique_ptr<CubeMapRenderPass> renderPass_;
	std::unique_ptr<vk::DescriptorSetManager> descriptorSetManager_;
	std::unique_ptr<vk::PipelineLayout> pipelineLayout_;
};