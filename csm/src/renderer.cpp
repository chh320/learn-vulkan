#include "renderer.h"
#include "Assets/Model.h"
#include "Assets/Scene.h"
#include "Assets/Texture.h"
#include "Assets/UserInterface.h"
#include "Utilities/Glm.h"
#include "Vulkan/Buffer.h"
#include "Vulkan/BufferUtil.h"
#include "Vulkan/GraphicsPipeline.h"
#include "Vulkan/Image.h"
#include "Vulkan/ImageMemoryBarrier.h"
#include "Vulkan/ImageView.h"
#include "Vulkan/PipelineLayout.h"
#include "Vulkan/SingleTimeCommands.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/Window.h"
#include <chrono>
#include <iostream>
#include <numeric>


Renderer::Renderer(const vk::WindowConfig& windowConfig, const VkPresentModeKHR presentMode, const bool enableValidationLayers)
    : vk::Application(windowConfig, presentMode, enableValidationLayers)
{
}

Renderer::~Renderer()
{
    depthPipeline_.reset();
    scenePipeline_.reset();
    DeleteSwapChain();
    ui_.reset();
    scene_.reset();
    camera_.reset();
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
    /*static auto startTime = std::chrono::high_resolution_clock::now();
    auto curTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(curTime - startTime).count();*/

    ubo.model = glm::scale(glm::mat4(1.f), glm::vec3(0.05f));
    // ubo.model = glm::rotate(ubo.model, time * glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = camera_->getViewMatrix();
    ubo.proj = camera_->getProjMatrix();
    ubo.cameraPos = camera_->getViewPos();

    if (GetMouseLeftDown()) {
        camera_->rotateByScreenX(camera_->getTarget(), GetMouseHorizontalMove() * 0.015);
        camera_->rotateByScreenY(camera_->getTarget(), GetMouseVerticalMove() * 0.01);
    } else if (GetMouseRightDown()) {
        camera_->moveCamera(GetMouseHorizontalMove(), GetMouseVerticalMove());
    }

    return ubo;
}

void Renderer::LoadScene()
{
    Assets::Model leaves = Assets::Model::LoadModel("../models/tree/MapleTreeLeaves.obj");
    Assets::Model stem = Assets::Model::LoadModel("../models/tree/MapleTreeStem.obj");
    Assets::Model floor = Assets::Model::LoadModel("../models/floor.obj");
    Assets::Model box = Assets::Model::LoadModel("../models/box.obj");

    std::vector<Assets::Model> models { floor, box, stem, leaves };
    std::vector<Assets::Texture> textures;
    textures.push_back(Assets::Texture::LoadTexture("../models/tree/maple_leaf.png", vk::SamplerConfig()));
    textures.push_back(Assets::Texture::LoadTexture("../models/tree/maple_leaf_Mask.png", vk::SamplerConfig()));
    textures.push_back(Assets::Texture::LoadTexture("../models/tree/maple_bark.png", vk::SamplerConfig()));

    scene_.reset(new Assets::Scene(CommandPool(), std::move(models), std::move(textures)));
}

void Renderer::Render(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    UpdateUi();
    UpdateLight();
    UpdateCascades();

#pragma region depthPass
    {
        std::array<VkClearValue, 1> clearValues = {};
        clearValues[0].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = depthPipeline_->RenderPass().Handle();
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = VkExtent2D { static_cast<int32_t>(SHADOWMAP_DIM), static_cast<int32_t>(SHADOWMAP_DIM) };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        VkViewport viewport {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<uint32_t>(SHADOWMAP_DIM);
        viewport.height = static_cast<uint32_t>(SHADOWMAP_DIM);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor {};
        scissor.offset = { 0, 0 };
        scissor.extent = VkExtent2D { static_cast<int32_t>(SHADOWMAP_DIM), static_cast<int32_t>(SHADOWMAP_DIM) };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        for (int32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
            renderPassInfo.framebuffer = depthFrameBuffer_[i].Handle();
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                const auto& scene = GetScene();
                VkDescriptorSet descriptorSets[] = { depthPipeline_->DescriptorSet(imageIndex) };
                VkBuffer vertexBuffers[] = { scene.VertexBuffer().Handle() };
                const VkBuffer indexBuffer = scene.IndexBuffer().Handle();
                VkDeviceSize offsets[] = { 0 };

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, depthPipeline_->Handle());
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, depthPipeline_->PipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

                uint32_t vertexOffset = 0;
                uint32_t indexOffset = 0;
                uint32_t modelCount = 0;
                glm::vec3 position[3] = {
                    glm::vec3(30.f, 0.f, -30.f),
                    glm::vec3(-60.f, 0.f, -10.f),
                    glm::vec3(0.f, 0.f, 40.f)
                };

                depthPipeline_->pushBlock.cascadedID = i;
                for (const auto& model : scene.Models()) {
                    const auto vertexCount = static_cast<uint32_t>(model.NumberOfVertices());
                    const auto indexCount = static_cast<uint32_t>(model.NumberOfIndices());

                    modelCount++;
                    depthPipeline_->pushBlock.modelID = modelCount;
                    if (modelCount > 2) {
                        for (int i = 0; i < 3; i++) {
                            depthPipeline_->pushBlock.position = position[i];
                            vkCmdPushConstants(commandBuffer, depthPipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                                0, sizeof(DepthPipeline::pushBlock), &depthPipeline_->pushBlock);
                            vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);
                        }
                    } else {
                        depthPipeline_->pushBlock.position = glm::vec3(0.f);
                        vkCmdPushConstants(commandBuffer, depthPipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                            0, sizeof(DepthPipeline::pushBlock), &depthPipeline_->pushBlock);
                        vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);
                    }

                    vertexOffset += vertexCount;
                    indexOffset += indexCount;
                }
            }
            vkCmdEndRenderPass(commandBuffer);
        }
    }
#pragma endregion

    {
        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { { 0.9f, 1.0f, 1.0f, 1.0f } };
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
            VkViewport viewport {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<uint32_t>(SwapChain().Extent().width);
            viewport.height = static_cast<uint32_t>(SwapChain().Extent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor {};
            scissor.offset = { 0, 0 };
            scissor.extent = SwapChain().Extent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            const auto& scene = GetScene();
            VkDescriptorSet descriptorSets[] = { scenePipeline_->DescriptorSet(imageIndex) };
            VkBuffer vertexBuffers[] = { scene.VertexBuffer().Handle() };
            const VkBuffer indexBuffer = scene.IndexBuffer().Handle();
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, scenePipeline_->Handle());
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, scenePipeline_->PipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            uint32_t vertexOffset = 0;
            uint32_t indexOffset = 0;
            uint32_t modelCount = 0;
            glm::vec3 position[3] = {
                glm::vec3(30.f, 0.f, -30.f),
                glm::vec3(-60.f, 0.f, -10.f),
                glm::vec3(0.f, 0.f, 40.f)
            };

            scenePipeline_->pushBlock.colorCascades = colorCascades;
            for (const auto& model : scene.Models()) {
                const auto vertexCount = static_cast<uint32_t>(model.NumberOfVertices());
                const auto indexCount = static_cast<uint32_t>(model.NumberOfIndices());

                modelCount++;
                scenePipeline_->pushBlock.modelID = modelCount;
                if (modelCount > 2) {
                    for (int i = 0; i < 3; i++) {
                        scenePipeline_->pushBlock.position = position[i];
                        vkCmdPushConstants(commandBuffer, scenePipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                            0, sizeof(ScenePipeline::pushBlock), &scenePipeline_->pushBlock);
                        vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);
                    }
                } else {
                    scenePipeline_->pushBlock.position = glm::vec3(0.f);
                    vkCmdPushConstants(commandBuffer, scenePipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0, sizeof(ScenePipeline::pushBlock), &scenePipeline_->pushBlock);
                    vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);
                }

                vertexOffset += vertexCount;
                indexOffset += indexCount;
            }

            UI().Draw(commandBuffer);
        }
        vkCmdEndRenderPass(commandBuffer);
    }

    CleanUpMouseStatus();
}

void Renderer::Prepare()
{
    // camera
    glm::vec3 viewPos = glm::vec3(5.18f, 2.6f, -6.5f);
    glm::vec3 target = glm::vec3(0.f);
    glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
    float fov = 45.f;
    float aspect = (float)SwapChain().Extent().width / (float)SwapChain().Extent().height;
    float zNear = 0.5f;
    float zFar = 200.f;
    camera_.reset(new Assets::Camera(viewPos, target, worldUp, fov, aspect, zNear, zFar));
    ui_.reset(new Assets::UserInterface(CommandPool(), GraphicsPipeline().RenderPass()));

    UpdateLight();

    cascadedDepthBuffer_.reset(new vk::DepthBuffer(CommandPool(), VkExtent2D { static_cast<int32_t>(SHADOWMAP_DIM), static_cast<int32_t>(SHADOWMAP_DIM) }, static_cast<int32_t>(SHADOW_MAP_CASCADE_COUNT), true));
    lightUniformBuffer_.reset(new Assets::UniformBuffer(Device(), lightUBO_));
    shadowUniformBuffer_.reset(new Assets::UniformBuffer(Device(), shadowUBO_));
    depthPipeline_.reset(new DepthPipeline(Device(), UniformBuffers(), GetLightUniformBuffer(), GetCascadedDepthBuffer(), GetScene()));
    scenePipeline_.reset(new ScenePipeline(Device(), UniformBuffers(), GetScene(), GraphicsPipeline().RenderPass(), GetCascadedDepthBuffer(), GetShadowUniformBuffer()));

    for (int32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        depthFrameBuffer_.emplace_back(i, depthPipeline_->RenderPass(), SHADOWMAP_DIM);
    }

    UpdateUi();
}

void Renderer::UpdateLight()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto curTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(curTime - startTime).count();

    float angle = glm::radians(time * 15.f);
    float radius = 20.f;
    lightPos = glm::vec3(std::cos(angle) * radius, radius, std::sin(angle) * radius);
}

void Renderer::UpdateCascades()
{
    float cascadeSplits[SHADOW_MAP_CASCADE_COUNT] /*= {0.02f, 0.12f, 0.45f, 1.f}*/;

    float nearClip = camera_->getNear();
    float farClip = camera_->getFar();
    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    // Calculate split depths based on view camera frustum
    // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
        float log = minZ * std::pow(ratio, p);
        float uniform = minZ + range * p;
        float d = cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
    }

    float lastSplitDist = 0.0;
    for (int32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float splitDist = cascadeSplits[i];

        glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f,  1.0f, 0.0f),
            glm::vec3( 1.0f,  1.0f, 0.0f),
            glm::vec3( 1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f, -1.0f, 0.0f),
            glm::vec3(-1.0f,  1.0f, 1.0f),
            glm::vec3( 1.0f,  1.0f, 1.0f),
            glm::vec3( 1.0f, -1.0f, 1.0f),
            glm::vec3(-1.0f, -1.0f, 1.0f),
        };

        // Project frustum corners into world space
        glm::mat4 invCam = glm::inverse(camera_->getProjMatrix() * camera_->getViewMatrix());
        for (uint32_t i = 0; i < 8; i++) {
            glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
            frustumCorners[i] = invCorner / invCorner.w;
        }

        for (uint32_t i = 0; i < 4; i++) {
            glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum sphere bound center
        float a2 = pow(glm::length(frustumCorners[2] - frustumCorners[0]), 2.0);
        float b2 = pow(glm::length(frustumCorners[6] - frustumCorners[4]), 2.0);
        float len = (splitDist - lastSplitDist) * range;
        float x = len * 0.5f + (a2 - b2) / (8.f * len);

        float zDistance = len - x;
        glm::vec3 sphereCenterVS = glm::vec3(0.f, 0.f, minZ + zDistance);
        glm::vec3 sphereCenterWS = camera_->getViewPos() + camera_->getViewDir() * sphereCenterVS.z;

        float sphereRadius = std::sqrtf(zDistance * zDistance + (a2 * 0.25f));

        // https://learn.microsoft.com/zh-cn/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps
        float worldUnitsPerPixel = sphereRadius * 2.f / (float)SHADOWMAP_DIM;
        sphereRadius = std::floor(sphereRadius / worldUnitsPerPixel) * worldUnitsPerPixel;
        glm::vec3 lightDir = normalize(lightPos);
        glm::mat4 shadowView = glm::lookAt(glm::vec3(0.f), -lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 sphereCenterLS = shadowView * glm::vec4(sphereCenterWS, 1.0f);
        sphereCenterLS -= glm::vec3(fmodf(sphereCenterLS.x, worldUnitsPerPixel), fmodf(sphereCenterLS.y, worldUnitsPerPixel), 0.f);
        glm::mat4 invShadowView = glm::inverse(shadowView);
        sphereCenterWS = glm::vec3(invShadowView * glm::vec4(sphereCenterLS, 1.f));

        // TODO : adjust backDistance to include all objects which are before lightPos.
        float backDistance = sphereRadius;
        glm::mat4 lightViewMatrix = glm::lookAt(sphereCenterWS + lightDir * backDistance, sphereCenterWS, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightOrthoMatrix = glm::ortho(-sphereRadius, sphereRadius, -sphereRadius, sphereRadius, 0.0f, sphereRadius * 2.f);

        shadowUBO_.splitDepth[i] = (camera_->getNear() + splitDist * clipRange) * -1.f;
        shadowUBO_.viewProjMatrix[i] = lightOrthoMatrix * lightViewMatrix;

        lightUBO_.viewProjMatrix[i] = shadowUBO_.viewProjMatrix[i];

        lastSplitDist = cascadeSplits[i];
    }
    shadowUBO_.lightDir = normalize(-lightPos);

    shadowUniformBuffer_->SetValue(shadowUBO_);
    lightUniformBuffer_->SetValue(lightUBO_);
}

void Renderer::UpdateUi()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 lastDisplaySize = io.DisplaySize;
    io.DisplaySize = ImVec2((float)SwapChain().Extent().width, (float)SwapChain().Extent().height);
    // io.DeltaTime = frameTimer;

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
    ImGui::SetNextWindowSize(ImVec2(200 * scale, 360 * scale), ImGuiCond_Always);
    ImGui::Begin("Cascaded Shadow Map", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(100.0f * scale);

    UI().text("cascaded shadow map exmaple");
    // UI().text("%.1d fps (%.2f ms)", lastFPS, (1000.0f / lastFPS));

    UI().checkbox("Color cascades", &colorCascades);

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

void Renderer::OnKey(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (key) {
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
        switch (action) {
        case GLFW_PRESS:
            mouseStatus_.lDown = true;
            break;
        case GLFW_RELEASE:
            mouseStatus_.lDown = false;
            break;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        switch (action) {
        case GLFW_PRESS:
            mouseStatus_.rDown = true;
            break;
        case GLFW_RELEASE:
            mouseStatus_.rDown = false;
            break;
        }
    }
}