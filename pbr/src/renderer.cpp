#include "renderer.h"
#include "Assets/Model.h"
#include "Assets/Scene.h"
#include "Assets/Texture.h"
#include "Assets/UserInterface.h"
#include "Utilities/Glm.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/BufferUtil.h"
#include "Vulkan/Image.h"
#include "Vulkan/ImageMemoryBarrier.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/PipelineLayout.h"
#include "Vulkan/SingleTimeCommands.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/GraphicsPipeline.h"
#include "Vulkan/Window.h"
#include <chrono>
#include <iostream>
#include <numeric>


Renderer::Renderer(const vk::WindowConfig& windowConfig, const VkPresentModeKHR presentMode, const bool enableValidationLayers) :
	vk::Application(windowConfig, presentMode, enableValidationLayers)
{
}

Renderer::~Renderer()
{
	brdflutPipeline_.reset();
	cubemapPipeline_.reset();
	skyboxPipeline_.reset();
	pbrPipeline_.reset();
	DeleteSwapChain();
	ui_.reset();
	scene_.reset();
	skybox_.reset();
	camera_.reset();
	shaderValuesUBO_.reset();
}

void Renderer::OnDeviceSet()
{
	Application::OnDeviceSet();

	LoadScene();
}

void Renderer::CreateSwapChain()
{
	Application::CreateSwapChain();

	static bool first = true;
	if (first) {
		Prepare();
		first = false;
	}



	camera_->setAspect((float)SwapChain().Extent().width / (float)SwapChain().Extent().height);
}

void Renderer::DeleteSwapChain()
{
	Application::DeleteSwapChain();
}

Assets::UniformBufferObject Renderer::GetUniformBufferObject(const VkExtent2D extent) const
{
	Assets::UniformBufferObject ubo = {};
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto curTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(curTime - startTime).count();

	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.view = camera_->getViewMatrix();
	ubo.proj = camera_->getProjMatrix();
	ubo.cameraPos = camera_->getViewPos();

	if (GetMouseLeftDown()) {
		camera_->rotateByScreenX(camera_->getTarget(), GetMouseHorizontalMove() * 0.015);
		camera_->rotateByScreenY(camera_->getTarget(), GetMouseVerticalMove() * 0.01);
	}
	else if (GetMouseRightDown()) {
		camera_->moveCamera(GetMouseHorizontalMove(), GetMouseVerticalMove());
	}

	return ubo;
}

void Renderer::LoadScene()
{
	Assets::Model helmet = Assets::Model::LoadModel("../models/helmet/helmet.obj");
	std::vector<Assets::Model> models{ helmet };
	std::vector<Assets::Texture> textures;
	textures.push_back(Assets::Texture::LoadTexture("../models/helmet/helmet_diffuse.tga", vk::SamplerConfig()));
	textures.push_back(Assets::Texture::LoadTexture("../models/helmet/helmet_emission.tga", vk::SamplerConfig()));
	textures.push_back(Assets::Texture::LoadTexture("../models/helmet/helmet_metalness.tga", vk::SamplerConfig()));
	textures.push_back(Assets::Texture::LoadTexture("../models/helmet/helmet_normal.tga", vk::SamplerConfig()));
	textures.push_back(Assets::Texture::LoadTexture("../models/helmet/helmet_occlusion.tga", vk::SamplerConfig()));
	textures.push_back(Assets::Texture::LoadTexture("../models/helmet/helmet_roughness.tga", vk::SamplerConfig()));
	
	/*std::vector<Assets::GltfModel> gltfModels;
	gltfModels.emplace_back(Device());
	gltfModels.back().LoadGLTFModel("./models/DamagedHelmet.gltf", 1.0f);

	scene_.reset(new Assets::Scene(CommandPool(), std::move(gltfModels)));*/
	scene_.reset(new Assets::Scene(CommandPool(), std::move(models), std::move(textures)));

	textures.clear();
	Assets::Model box = Assets::Model::LoadModel("../models/box.obj");
	std::vector<Assets::Model> skybox { box };
	textures.push_back(Assets::Texture::LoadTexture("../textures/blue_photo_studio_4k.hdr", vk::SamplerConfig()));

	skybox_.reset(new Assets::Scene(CommandPool(), std::move(skybox), std::move(textures)));
}

void Renderer::Render(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	UpdateUi();
	UpdateUBO();

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = GraphicsPipeline().RenderPass().Handle();
	renderPassInfo.framebuffer = SwapChainFrameBuffer(imageIndex).Handle();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = SwapChain().Extent();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		const auto& scene = GetScene();

		VkDescriptorSet descriptorSets[] = { pbrPipeline_->DescriptorSet(imageIndex) };
		VkBuffer vertexBuffers[] = { scene.VertexBuffer().Handle() };
		const VkBuffer indexBuffer = scene.IndexBuffer().Handle();
		VkDeviceSize offsets[] = { 0 };

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<uint32_t> (SwapChain().Extent().width);
		viewport.height = static_cast<uint32_t> (SwapChain().Extent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SwapChain().Extent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline_->Handle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline_->PipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		for (const auto& model : scene.Models())
		{
			const auto vertexCount = static_cast<uint32_t>(model.NumberOfVertices());
			const auto indexCount = static_cast<uint32_t>(model.NumberOfIndices());

			vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);

			vertexOffset += vertexCount;
			indexOffset += indexCount;
		}

		{
			const auto& scene = GetSkybox();

			VkDescriptorSet descriptorSets[] = { skyboxPipeline_->DescriptorSet(imageIndex) };
			VkBuffer vertexBuffers[] = { scene.VertexBuffer().Handle() };
			const VkBuffer indexBuffer = scene.IndexBuffer().Handle();

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline_->Handle());
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline_->PipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			uint32_t vertexOffset = 0;
			uint32_t indexOffset = 0;

			for (const auto& model : scene.Models())
			{
				const auto vertexCount = static_cast<uint32_t>(model.NumberOfVertices());
				const auto indexCount = static_cast<uint32_t>(model.NumberOfIndices());

				vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);

				vertexOffset += vertexCount;
				indexOffset += indexCount;
			}

			UI().Draw(commandBuffer);
		}
	}
	vkCmdEndRenderPass(commandBuffer);

	CleanUpMouseStatus();
}

void Renderer::Prepare() {
	// camera
	glm::vec3 viewPos = glm::vec3(0.f, 0.f, 5.f);
	glm::vec3 target = glm::vec3(0.f);
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
	float fov = 60.f;
	float aspect = (float)SwapChain().Extent().width / (float)SwapChain().Extent().height;
	float zNear = 0.1f;
	float zFar = 256.0f;
	camera_.reset(new Assets::Camera(viewPos, target, worldUp, fov, aspect, zNear, zFar));
	shaderValuesUBO_.reset(new Assets::UniformBuffer(Device()));

	// cubemap
	GenerateCubemaps();
	GenratateBRDFLUT();

	pbrPipeline_.reset(new PbrPipeline(Device(), UniformBuffers(), GetShaderValuesUBO(), GetScene(), GraphicsPipeline().RenderPass(),
		GetIrradianceMap(), GetPrefilteredMap(), GetBrdfLutMap()));
	skyboxPipeline_.reset(new SkyBoxPipeline(Device(), UniformBuffers(), GetSkybox(), GraphicsPipeline().RenderPass(), GetPrefilteredMap()));

	ui_.reset(new Assets::UserInterface(CommandPool(), GraphicsPipeline().RenderPass()));
	UpdateUi();

}

void Renderer::GenerateCubemaps() {
	enum Target { IRRADIANCE = 0, PREFILTEREDENV = 1 };
	for (uint32_t target = 0; target < PREFILTEREDENV + 1; target++) {
		uint32_t dim;
		VkFormat format;

		auto tStart = std::chrono::high_resolution_clock::now();

		switch (target) {
		case IRRADIANCE:
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
			dim = 64;
			break;
		case PREFILTEREDENV:
			format = VK_FORMAT_R16G16B16A16_SFLOAT;
			dim = 512;
			break;
		}

		const int32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

		std::unique_ptr<Assets::TextureCubeImage> cubeTex = std::make_unique<Assets::TextureCubeImage>(CommandPool(), dim, format, numMips);
		struct Offscreen {
			std::unique_ptr<vk::Image> image;
			std::unique_ptr<vk::ImageView> view;
			std::unique_ptr<vk::DeviceMemory> memory;
			std::unique_ptr<CubemapFrameBuffer> framebuffer;
		} offscreen;

		// Create offscreen framebuffer
		cubemapPipeline_.reset(new CubeMapPipeline(Device(), GetSkybox(), format, target));

		offscreen.image.reset(new vk::Image(Device(), VkExtent2D{ dim, dim }, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 1, 1));
		offscreen.memory.reset(new vk::DeviceMemory(offscreen.image->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		offscreen.view.reset( new vk::ImageView(Device(), offscreen.image->Handle(), offscreen.image->Format(), VK_IMAGE_ASPECT_COLOR_BIT));
		offscreen.framebuffer.reset( new CubemapFrameBuffer(*(offscreen.view), cubemapPipeline_->RenderPass(), dim));

		offscreen.image->TransitionImageLayout(CommandPool(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);
	
		// render cubemap
		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = cubemapPipeline_->RenderPass().Handle();
		renderPassBeginInfo.framebuffer = offscreen.framebuffer->Handle();
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

		std::vector<glm::mat4> matrices = {
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		VkViewport viewport{};
		viewport.width = (float)dim;
		viewport.height = (float)dim;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.extent.width = dim;
		scissor.extent.height = dim;

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = numMips;
		subresourceRange.layerCount = 6;

		cubeTex->Image().TransitionImageLayout(CommandPool(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, numMips, 6);
	
		for (uint32_t m = 0; m < numMips; m++) {
			for (uint32_t f = 0; f < 6; f++) {
				vk::SingleTimeCommands::Submit(CommandPool(), [&](VkCommandBuffer commandBuffer) 
					{
						viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
						viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
						vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
						vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

						vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
						{
							switch (target)
							{
							case IRRADIANCE:
								cubemapPipeline_->pushBlockIrradiance.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
								vkCmdPushConstants(commandBuffer, cubemapPipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
									sizeof(CubeMapPipeline::PushBlockIrradiance), &cubemapPipeline_->pushBlockIrradiance);
								break;
							case PREFILTEREDENV:
								cubemapPipeline_->pushBlockPrefilterEnv.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
								cubemapPipeline_->pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);
								vkCmdPushConstants(commandBuffer, cubemapPipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
									sizeof(CubeMapPipeline::PushBlockPrefilterEnv), &cubemapPipeline_->pushBlockPrefilterEnv);
								break;
							}

							const auto& scene = GetSkybox();
							VkBuffer vertexBuffers[] = { scene.VertexBuffer().Handle() };
							const VkBuffer indexBuffer = scene.IndexBuffer().Handle();
							VkDeviceSize offsets[] = { 0 };
							VkDescriptorSet descriptorSets[] = { cubemapPipeline_->DescriptorSet(0) };
							
							vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipeline_->Handle());
							vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cubemapPipeline_->PipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
							vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
							vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

							uint32_t vertexOffset = 0;
							uint32_t indexOffset = 0;

							for (const auto& model : scene.Models())
							{
								const auto vertexCount = static_cast<uint32_t>(model.NumberOfVertices());
								const auto indexCount = static_cast<uint32_t>(model.NumberOfIndices());

								vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);

								vertexOffset += vertexCount;
								indexOffset += indexCount;
							}
						}
						vkCmdEndRenderPass(commandBuffer);

						// change offscreen.image from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
						{
							VkImageMemoryBarrier imageMemoryBarrier{};
							imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							imageMemoryBarrier.image = offscreen.image->Handle();
							imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
							imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
							imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
							vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
						}

						// copy offscreen to cubemap
						VkImageCopy copyRegion{};

						copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						copyRegion.srcSubresource.baseArrayLayer = 0;
						copyRegion.srcSubresource.mipLevel = 0;
						copyRegion.srcSubresource.layerCount = 1;
						copyRegion.srcOffset = { 0, 0, 0 };

						copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						copyRegion.dstSubresource.baseArrayLayer = f;
						copyRegion.dstSubresource.mipLevel = m;
						copyRegion.dstSubresource.layerCount = 1;
						copyRegion.dstOffset = { 0, 0, 0 };

						copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
						copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
						copyRegion.extent.depth = 1;

						vkCmdCopyImage(commandBuffer, offscreen.image->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							cubeTex->Image().Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

						// change offscreen.image from VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						{
							VkImageMemoryBarrier imageMemoryBarrier{};
							imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
							imageMemoryBarrier.image = offscreen.image->Handle();
							imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
							imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
							imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
							imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
							vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
						}
					});
			}
		}
		cubeTex->Image().TransitionImageLayout(CommandPool(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, numMips, 6);

		switch (target)
		{
		case IRRADIANCE:
			irradianceMap_ = std::move(cubeTex);
			break;
		case PREFILTEREDENV:
			prefilterMap_ = std::move(cubeTex);
			shaderValuesParams_.prefilteredCubeMipLevels = numMips;
			break;
		}

		offscreen.framebuffer.reset();
		offscreen.image.reset();
		offscreen.memory.reset();
		offscreen.view.reset();

		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

		switch (target)
		{
		case IRRADIANCE:
			std::cout << "\nGenerating IRRADIANCE cubemap took " << tDiff << " ms\n";
			break;
		case PREFILTEREDENV:
			std::cout << "Generating PREFILTEREDENV cubemap took " << tDiff << " ms\n";
			break;
		}
	}
}

void Renderer::GenratateBRDFLUT() {
	auto tStart = std::chrono::high_resolution_clock::now();

	const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
	const int32_t dim = 512;

	brdfLut_.reset(new Assets::TextureImage(Device(), dim, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));
	brdflutPipeline_.reset(new BrdfLutPipeline(Device(), format));

	std::array<VkImageView, 1> attachments =
	{
		brdfLut_->ImageView().Handle()
	};

	VkFramebufferCreateInfo framebufferCI{};
	framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCI.renderPass = brdflutPipeline_->RenderPass().Handle();
	framebufferCI.attachmentCount = attachments.size();
	framebufferCI.pAttachments = attachments.data();
	framebufferCI.width = dim;
	framebufferCI.height = dim;
	framebufferCI.layers = 1;

	VkFramebuffer framebuffer;
	vk::Check(vkCreateFramebuffer(Device().Handle(), &framebufferCI, nullptr, &framebuffer), "create brdflut's framebuffer");

	// Render
	VkClearValue clearValues[1];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = brdflutPipeline_->RenderPass().Handle();
	renderPassBeginInfo.renderArea.extent.width = dim;
	renderPassBeginInfo.renderArea.extent.height = dim;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = framebuffer;

	vk::SingleTimeCommands::Submit(CommandPool(), [&](VkCommandBuffer commandBuffer)
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.width = (float)dim;
			viewport.height = (float)dim;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.extent.width = dim;
			scissor.extent.height = dim;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, brdflutPipeline_->Handle());
			vkCmdDraw(commandBuffer, 3, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffer);

		});

	vkDestroyFramebuffer(Device().Handle(), framebuffer, nullptr);
	
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating BRDF LUT took " << tDiff << " ms\n";
}

void Renderer::UpdateUi() {
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 lastDisplaySize = io.DisplaySize;
	io.DisplaySize = ImVec2((float)SwapChain().Extent().width, (float)SwapChain().Extent().height);
	//io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(mouseStatus_.lastX, mouseStatus_.lastY);
	io.MouseDown[0] = mouseStatus_.lDown;
	io.MouseDown[1] = mouseStatus_.rDown;

	UI().Pipeline().pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	UI().Pipeline().pushConstBlock.translate = glm::vec2(-1.0f);

	bool updateShaderParams = false;
	bool updateCBs = false;
	float scale = 1.0f;

	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(200 * scale,  360 * scale), ImGuiCond_Always);
	ImGui::Begin("Vulkan PBR", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::PushItemWidth(100.0f * scale);

	UI().text("my vulkan pbr shader");
	//UI().text("%.1d fps (%.2f ms)", lastFPS, (1000.0f / lastFPS));

	if (UI().header("Debug view")) {
		const std::vector<std::string> debugNamesInputs = {
			"none", "Base color", "Normal", "Occlusion", "Emissive", "Metallic", "Roughness"
		};
		if (UI().combo("Inputs", &debugViewInputs, debugNamesInputs)) {
			shaderValuesParams_.debugViewInputs = static_cast<float>(debugViewInputs);
			updateShaderParams = true;
		}
		const std::vector<std::string> debugNamesEquation = {
			"none", "Diff (l,n)", "F (l,h)", /*"G (l,v,h)", "D (h)",*/ "Specular"
		};
		if (UI().combo("PBR equation", &debugViewEquation, debugNamesEquation)) {
			shaderValuesParams_.debugViewEquation = static_cast<float>(debugViewEquation);
			updateShaderParams = true;
		}
	}

	ImGui::PopItemWidth();
	ImGui::End();
	ImGui::Render();

	ImDrawData* imDrawData = ImGui::GetDrawData();

	// Check if ui buffers need to be recreated
	if (imDrawData) {
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		bool updateBuffers = (vertexBufferSize != UI().vertexBufferSize || indexBufferSize != UI().indexBufferSize);
		if (updateBuffers) {
			vkDeviceWaitIdle(Device().Handle());
			UI().vertexBuffer.reset(new vk::Buffer(Device(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
			UI().indexBuffer.reset(new vk::Buffer(Device(), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
			UI().vertexBufferMemory.reset(new vk::DeviceMemory(UI().vertexBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));
			UI().indexBufferMemory.reset(new vk::DeviceMemory(UI().indexBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)));
			UI().vertexBufferSize = vertexBufferSize;
			UI().indexBufferSize = indexBufferSize;
		}

		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)UI().vertexBufferMemory->Map(0, vertexBufferSize);
		ImDrawIdx* idxDst = (ImDrawIdx*)UI().indexBufferMemory->Map(0, indexBufferSize);
		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}
		UI().vertexBufferMemory->Unmap();
		UI().indexBufferMemory->Unmap();
	}
}

void Renderer::UpdateUBO()
{
	shaderValuesUBO_->SetValue(shaderValuesParams_);
}

void Renderer::OnKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			Window().Close();
		default:
			break;
		}
	}
}

void Renderer::OnScroll(double xoffset, double yoffset)
{
	camera_->zoom(yoffset > 0.0 ? 1.0 : -1.0);
}

void Renderer::OnCursorPosition(double xpos, double ypos)
{
	if (mouseStatus_.lDown || mouseStatus_.rDown) {
		if (mouseStatus_.lastX != xpos)
			mouseStatus_.horizontalMove = xpos - mouseStatus_.lastX;
		if (mouseStatus_.lastY != ypos)
			mouseStatus_.verticalMove = ypos - mouseStatus_.lastY;
	}
	mouseStatus_.lastX = xpos;
	mouseStatus_.lastY = ypos;
}

void Renderer::OnMouseButton(int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		switch (action)
		{
		case GLFW_PRESS: 
			mouseStatus_.lDown = true; break;
		case GLFW_RELEASE:
			mouseStatus_.lDown = false; break;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		switch (action)
		{
		case GLFW_PRESS:
			mouseStatus_.rDown = true; break;
		case GLFW_RELEASE:
			mouseStatus_.rDown = false; break;
		}
	}
}