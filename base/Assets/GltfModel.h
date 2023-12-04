#pragma once
#ifndef GLTFMODEL_H
#define GLTFMODEL_H

//#include "tiny_gltf.h"

#include "Assets/Vertex.h"
#include "Assets/Texture.h"
#include "Assets/TextureImage.h"
#include "Assets/UniformBuffer.h"
#include "Vulkan/Device.h"
#include "Vulkan/Sampler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>
#include <vector>

#define MAX_NUM_JOINTS 128u

namespace Assets
{
	struct Node;

	struct BoundingBox {
		glm::vec3 min;
		glm::vec3 max;
		bool valid = false;
		BoundingBox();
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox getAABB(glm::mat4 m);
	};

	struct Material {
		enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
		AlphaMode alphaMode = ALPHAMODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		glm::vec4 emissiveFactor = glm::vec4(1.0f);
		int32_t baseColorTextureId;
		int32_t metallicRoughnessTextureId;
		int32_t normalTextureId;
		int32_t occlusionTextureId;
		int32_t emissiveTextureId;
		bool doubleSided = false;
		struct TexCoordSets {
			uint8_t baseColor = 0;
			uint8_t metallicRoughness = 0;
			uint8_t specularGlossiness = 0;
			uint8_t normal = 0;
			uint8_t occlusion = 0;
			uint8_t emissive = 0;
		} texCoordSets;
		struct Extension {
			int32_t specularGlossinessTextureId;
			int32_t diffuseTextureId;
			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			glm::vec3 specularFactor = glm::vec3(0.0f);
		} extension;
		struct PbrWorkflows {
			bool metallicRoughness = true;
			bool specularGlossiness = false;
		} pbrWorkflows;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	};

	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t vertexCount;
		Material& material;
		bool hasIndices;
		BoundingBox bb;
		Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material);
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	struct Mesh {
		std::vector<Primitive*> primitives;
		BoundingBox bb;
		BoundingBox aabb;

		struct UniformBlock {
			glm::mat4 matrix;
			glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
			float jointcount{ 0 };
		} uniformBlock;

		Mesh(const vk::Device& device, glm::mat4 matrix);
		~Mesh();

		void setBoundingBox(glm::vec3 min, glm::vec3 max);
		Assets::UniformBuffer& UniformBuffer() { return *ubo_; }

		const vk::Device& device_;
		std::unique_ptr<Assets::UniformBuffer> ubo_;
	};

	struct Skin {
		std::string name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node*> joints;
	};

	struct Node {
		Node* parent;
		uint32_t index;
		std::vector<Node*> children;
		glm::mat4 matrix;
		std::string name;
		Mesh* mesh;
		Skin* skin;
		int32_t skinIndex = -1;
		glm::vec3 translation{};
		glm::vec3 scale{ 1.0f };
		glm::quat rotation{};
		BoundingBox bvh;
		BoundingBox aabb;
		glm::mat4 localMatrix();
		glm::mat4 getMatrix();
		void update();
		~Node();
	};

	struct AnimationChannel {
		enum PathType { TRANSLATION, ROTATION, SCALE };
		PathType path;
		Node* node;
		uint32_t samplerIndex;
	};

	struct AnimationSampler {
		enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct Animation {
		std::string name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	class GltfModel final
	{
	public:

		struct Dimensions {
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
		} dimensions;

		void LoadGLTFModel(const std::string& filename, const float scale);
		
		GltfModel& operator = (const GltfModel&) = delete;
		GltfModel& operator = (GltfModel&&) = delete;

		GltfModel() = default;
		GltfModel(const GltfModel&) = default;
		GltfModel(GltfModel&&) = default;
		~GltfModel();

		GltfModel(const vk::Device& device);

		const std::vector<GltfVertex>& Vertices() const { return vertices_; }
		const std::vector<uint32_t>& Indices() const { return indices_; }
		const std::vector<Assets::GltfTexture>& Textures() const { return textures_; }
		const std::vector<vk::SamplerConfig>& Sampler() const { return textureSamplers_; }
		const vk::Device& Device() const { return device_; }

		uint32_t NumberOfVertices() const { return static_cast<uint32_t>(vertices_.size()); }
		uint32_t NumberOfIndices() const { return static_cast<uint32_t>(indices_.size()); }

	private:
		void loadTextureSamplers(tinygltf::Model& gltfModel);
		void loadTextures(tinygltf::Model& gltfModel);
		void loadMaterials(tinygltf::Model& gltfModel);
		void loadNode(Assets::Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, float globalscale);
		void loadAnimations(tinygltf::Model& gltfModel);
		void loadSkins(tinygltf::Model& gltfModel);
		VkFilter getVkFilterMode(int32_t filterMode);
		VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
		void getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount);
		Node* nodeFromIndex(uint32_t index);
		Node* findNode(Node* parent, uint32_t index);
		void getSceneDimensions();
		void calculateBoundingBox(Node* node, Node* parent);

		GltfModel(std::vector<GltfVertex>&& vertices, std::vector<uint32_t>&& indices);

		std::vector<GltfVertex> vertices_;
		std::vector<uint32_t> indices_;
		std::vector<vk::SamplerConfig> textureSamplers_;
		std::vector<Assets::GltfTexture> textures_;
		std::vector<Assets::Material> materials_;
		std::vector<Node*> nodes_;
		std::vector<Node*> linearNodes_;
		std::vector<Animation> animations_;
		std::vector<Skin*> skins_;
		glm::mat4 aabb_;

		const vk::Device& device_;
	};

}

#endif // 