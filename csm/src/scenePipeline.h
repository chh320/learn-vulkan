#pragma once
#include "Vulkan/VkConfig.h"
#include "Vulkan/Device.h"
#include "Vulkan/DescriptorSetManager.h"
#include "Vulkan/PipelineLayout.h"
#include "Vulkan/RenderPass.h"
#include "Assets/UniformBuffer.h"
#include "Assets/Scene.h"
#include <memory>

class ScenePipeline final {
public:
	VULKAN_NON_COPIABLE(ScenePipeline)

	struct PushBlock {
		glm::vec3 position;
		int32_t modelID;
		int32_t colorCascades;
	}pushBlock;

	ScenePipeline(
		const vk::Device& device,
		const std::vector<Assets::UniformBuffer>& uniformBuffers,
		const Assets::Scene& scene,
		const vk::RenderPass& renderPass,
		const vk::DepthBuffer& depthBuffer,
		const Assets::UniformBuffer& shadowUBO);
	~ScenePipeline();

	VkDescriptorSet DescriptorSet(uint32_t index) const;
	const vk::PipelineLayout& PipelineLayout() const { return *pipelineLayout_; }
	const vk::Device& Device() const { return device_; }

private:
	const vk::Device& device_;

	VULKAN_HANDLE(VkPipeline, pipeline_)

	std::unique_ptr<vk::DescriptorSetManager> descriptorSetManager_;
	std::unique_ptr<vk::PipelineLayout> pipelineLayout_;
};