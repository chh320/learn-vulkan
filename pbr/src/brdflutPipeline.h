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
#include "brdflutRenderPass.h"
#include <memory>
#include <vector>

#define M_PI 3.14159265358979323846

class BrdfLutPipeline final
{
public:

	VULKAN_NON_COPIABLE(BrdfLutPipeline)

	BrdfLutPipeline(
		const vk::Device& device,
		VkFormat format);
	~BrdfLutPipeline();

	VkDescriptorSet DescriptorSet(uint32_t index) const;
	const vk::PipelineLayout& PipelineLayout() const { return *pipelineLayout_; }
	const BrdfLutRenderPass& RenderPass() const { return *renderPass_; }
	const vk::Device& Device() const { return device_; }

private:
	const vk::Device& device_;

	VULKAN_HANDLE(VkPipeline, pipeline_)

	std::unique_ptr<BrdfLutRenderPass> renderPass_;
	std::unique_ptr<vk::DescriptorSetManager> descriptorSetManager_;
	std::unique_ptr<vk::PipelineLayout> pipelineLayout_;
};