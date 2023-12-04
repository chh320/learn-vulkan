#include "Assets/Model.h"
#include "Utilities/Console.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <stdexcept>

using namespace glm;

namespace std
{
	template<> struct hash<Assets::Vertex> final
	{
		size_t operator()(Assets::Vertex const& vertex) const noexcept
		{
			return
				Combine(hash<vec3>()(vertex.Position),
					Combine(hash<vec3>()(vertex.Normal),
						hash<vec2>()(vertex.TexCoord)));
		}

	private:

		static size_t Combine(size_t hash0, size_t hash1)
		{
			return hash0 ^ (hash1 + 0x9e3779b9 + (hash0 << 6) + (hash0 >> 2));
		}
	};
}

namespace Assets {

	Model Model::LoadModel(const std::string& filename)
	{
		std::cout << "- loading '" << filename << "'... " << std::flush;

		const auto timer = std::chrono::high_resolution_clock::now();

		tinyobj::attrib_t tmpAttrib;
		std::vector<tinyobj::shape_t> tmpShapes;
		std::vector<tinyobj::material_t> tmpMaterials;
		std::string err;

		if (!tinyobj::LoadObj(&tmpAttrib, &tmpShapes, &tmpMaterials, &err, filename.c_str())) {
			throw std::runtime_error(err);
		}

		// Geometry
		const auto& objAttrib = tmpAttrib;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::unordered_map<Vertex, uint32_t> uniqueVertices(objAttrib.vertices.size());
		size_t faceId = 0;

		for (const auto& shape : tmpShapes)
		{
			const auto& mesh = shape.mesh;

			for (const auto& index : mesh.indices)
			{
				Vertex vertex = {};

				vertex.Position =
				{
					objAttrib.vertices[3 * index.vertex_index + 0],
					objAttrib.vertices[3 * index.vertex_index + 1],
					objAttrib.vertices[3 * index.vertex_index + 2],
				};

				if (!objAttrib.normals.empty())
				{
					vertex.Normal =
					{
						objAttrib.normals[3 * index.normal_index + 0],
						objAttrib.normals[3 * index.normal_index + 1],
						objAttrib.normals[3 * index.normal_index + 2]
					};
				}

				if (!objAttrib.texcoords.empty())
				{
					vertex.TexCoord =
					{
						fmod(objAttrib.texcoords[2 * index.texcoord_index + 0], 1.0),
						1.0 - fmod(objAttrib.texcoords[2 * index.texcoord_index + 1], 1.0)
					};
				}

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		// If the model did not specify normals, then create smooth normals that conserve the same number of vertices.
		// Using flat normals would mean creating more vertices than we currently have, so for simplicity and better visuals we don't do it.
		// See https://stackoverflow.com/questions/12139840/obj-file-averaging-normals.
		if (objAttrib.normals.empty())
		{
			std::vector<vec3> normals(vertices.size());

			for (size_t i = 0; i < indices.size(); i += 3)
			{
				const auto normal = normalize(cross(
					vec3(vertices[indices[i + 1]].Position) - vec3(vertices[indices[i]].Position),
					vec3(vertices[indices[i + 2]].Position) - vec3(vertices[indices[i]].Position)));

				vertices[indices[i + 0]].Normal += normal;
				vertices[indices[i + 1]].Normal += normal;
				vertices[indices[i + 2]].Normal += normal;
			}

			for (auto& vertex : vertices)
			{
				vertex.Normal = normalize(vertex.Normal);
			}
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();

		std::cout << "(" << objAttrib.vertices.size() << " vertices, " << uniqueVertices.size() << " unique vertices)" ;
		std::cout << elapsed << "s" << std::endl;

		return Model(std::move(vertices), std::move(indices));
	}

	

	void Model::Transform(const mat4& transform)
	{
		const auto transformIT = inverseTranspose(transform);

		for (auto& vertex : vertices_)
		{
			vertex.Position = transform * vec4(vertex.Position, 1);
			vertex.Normal = transformIT * vec4(vertex.Normal, 0);
		}
	}

	Model::Model(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices) :
		vertices_(std::move(vertices)),
		indices_(std::move(indices))
	{
	}

}
