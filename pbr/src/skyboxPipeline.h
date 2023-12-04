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
#include "Vulkan/Sampler.h"
#include "Vulkan/ImageView.h"
#include "Assets/Scene.h"
#include "Assets/Vertex.h"
#include "Assets/UniformBuffer.h"
#include "Assets/TextureImage.h"
#include <memory>
#include <vector>

class SkyBoxPipeline final
{
public:

	VULKAN_NON_COPIABLE(SkyBoxPipeline)
	
	SkyBoxPipeline(
		const vk::Device& device,
		const std::vector<Assets::UniformBuffer>& uniformBuffers,
		const Assets::Scene& scene,
		const vk::RenderPass& renderPass,
		const Assets::TextureCubeImage& cubeMap);
	~SkyBoxPipeline();

	VkDescriptorSet DescriptorSet(uint32_t index) const;
	const vk::PipelineLayout& PipelineLayout() const { return *pipelineLayout_; }
	const vk::Device& Device() const { return device_; }

private:
	const vk::Device& device_;

	VULKAN_HANDLE(VkPipeline, pipeline_)

	std::unique_ptr<vk::DescriptorSetManager> descriptorSetManager_;
	std::unique_ptr<vk::PipelineLayout> pipelineLayout_;
};