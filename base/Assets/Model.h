#pragma once
#include "Assets/Vertex.h"
#include <memory>
#include <string>
#include <vector>

namespace Assets
{
	class Model final
	{
	public:

		static Model LoadModel(const std::string& filename);
		Model& operator = (const Model&) = delete;
		Model& operator = (Model&&) = delete;

		Model() = default;
		Model(const Model&) = default;
		Model(Model&&) = default;
		~Model() = default;

		void Transform(const glm::mat4& transform);

		const std::vector<Vertex>& Vertices() const { return vertices_; }
		const std::vector<uint32_t>& Indices() const { return indices_; }


		uint32_t NumberOfVertices() const { return static_cast<uint32_t>(vertices_.size()); }
		uint32_t NumberOfIndices() const { return static_cast<uint32_t>(indices_.size()); }

	private:

		Model(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

		std::vector<Vertex> vertices_;
		std::vector<uint32_t> indices_;
	};

}
