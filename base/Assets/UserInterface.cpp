#include "Assets/UserInterface.h"

namespace Assets {
	UserInterface::UserInterface(
		vk::CommandPool& commandPool,
		const vk::RenderPass& renderPass)
	{
		const auto& device = commandPool.Device();

		ImGui::CreateContext();

		// font texture loading
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData;
		int texWidth, texHeight;

		io.Fonts->AddFontFromFileTTF("./textures/fonts/Roboto-Medium.ttf", 16.f);
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		fontTexture_.reset(new Assets::TextureImage(commandPool, Assets::Texture(texWidth, texHeight, 1, fontData)));
		
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 0.0f;
		style.WindowBorderSize = 0.0f;
		style.WindowRounding = 7.f;

		pipeline_.reset(new Assets::UiPipeline(device, renderPass, FontTexture()));
	}

	void UserInterface::Draw(VkCommandBuffer cmdBuffer)
	{
		VkDescriptorSet descriptorSets[] = { pipeline_->DescriptorSet(0) };

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->Handle());
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_->PipelineLayout().Handle(), 0, 1, descriptorSets, 0, nullptr);

		const VkDeviceSize offsets[1] = { 0 };
		VkBuffer vertexBuffers[] = { vertexBuffer->Handle() };
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdPushConstants(cmdBuffer, pipeline_->PipelineLayout().Handle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UiPipeline::PushConstBlock), &pipeline_->pushConstBlock);

		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;
		for (int32_t j = 0; j < imDrawData->CmdListsCount; j++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[j];
			for (int32_t k = 0; k < cmd_list->CmdBuffer.Size; k++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[k];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(cmdBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(cmdBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}

	UserInterface::~UserInterface()
	{
		fontTexture_.reset();
		pipeline_.reset();
	}
	
	bool UserInterface::header(const char* caption) {
		return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
	}
	bool UserInterface::slider(const char* caption, float* value, float min, float max) {
		return ImGui::SliderFloat(caption, value, min, max);
	}
	bool UserInterface::combo(const char* caption, int32_t* itemindex, std::vector<std::string> items) {
		if (items.empty()) {
			return false;
		}
		std::vector<const char*> charitems;
		charitems.reserve(items.size());
		for (size_t i = 0; i < items.size(); i++) {
			charitems.push_back(items[i].c_str());
		}
		uint32_t itemCount = static_cast<uint32_t>(charitems.size());
		return ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
	}
	bool UserInterface::combo(const char* caption, std::string& selectedkey, std::map<std::string, std::string> items) {
		bool selectionChanged = false;
		if (ImGui::BeginCombo(caption, selectedkey.c_str())) {
			for (auto it = items.begin(); it != items.end(); ++it) {
				const bool isSelected = it->first == selectedkey;
				if (ImGui::Selectable(it->first.c_str(), isSelected)) {
					selectionChanged = it->first != selectedkey;
					selectedkey = it->first;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		return selectionChanged;
	}
	bool UserInterface::button(const char* caption) {
		return ImGui::Button(caption);
	}
	void UserInterface::text(const char* formatstr, ...) {
		va_list args;
		va_start(args, formatstr);
		ImGui::TextV(formatstr, args);
		va_end(args);
	}
}