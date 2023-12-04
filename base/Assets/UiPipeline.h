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

namespace Assets {
	class UiPipeline final {
	public:

		VULKAN_NON_COPIABLE(UiPipeline)

		UiPipeline(
			const vk::Device& device,
			const vk::RenderPass& renderPass,
			const Assets::TextureImage& fontTexture);
		~UiPipeline();

		VkDescriptorSet DescriptorSet(uint32_t index) const;
		const vk::PipelineLayout& PipelineLayout() const { return *pipelineLayout_; }
		const vk::Device& Device() const { return device_; }

		struct PushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

	private:
		const vk::Device& device_;

		VULKAN_HANDLE(VkPipeline, pipeline_)

		std::unique_ptr<vk::DescriptorSetManager> descriptorSetManager_;
		std::unique_ptr<vk::PipelineLayout> pipelineLayout_;
	};
}