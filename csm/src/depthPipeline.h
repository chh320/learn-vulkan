#pragma once
#include "Vulkan/VkConfig.h"
#include "Vulkan/Device.h"
#include "Vulkan/DescriptorSetManager.h"
#include "Vulkan/PipelineLayout.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/Buffer.h"
#include "Assets/UniformBuffer.h"
#include "Assets/Scene.h"
#include "depthRenderPass.h"
#include <memory>
#include <array>

#define SHADOW_MAP_CASCADE_COUNT 4
#define SHADOWMAP_DIM 4096

class DepthPipeline final {
public:
	VULKAN_NON_COPIABLE(DepthPipeline)

	struct PushBlock {
		glm::vec3 position;
		int32_t modelID;
		int32_t cascadedID;
	}pushBlock;

	DepthPipeline(
		const vk::Device& device,
		const std::vector<Assets::UniformBuffer>& uniformBuffers,
		const Assets::UniformBuffer& lightUBO,
		const vk::DepthBuffer& depthBuffer,
		const Assets::Scene& scene);
	~DepthPipeline();

	VkDescriptorSet DescriptorSet(uint32_t index) const;
	const vk::PipelineLayout& PipelineLayout() const { return *pipelineLayout_; }
	const vk::Device& Device() const { return device_; }
	const DepthRenderPass& RenderPass() const { return *renderPass_; }

private:
	const vk::Device& device_;

	VULKAN_HANDLE(VkPipeline, pipeline_)

	std::unique_ptr<vk::DescriptorSetManager> descriptorSetManager_;
	std::unique_ptr<vk::PipelineLayout> pipelineLayout_;
	std::unique_ptr<DepthRenderPass> renderPass_;
};