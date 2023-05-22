#pragma once

#include "meshes/raw_mesh.h"
#include "gfx/images/raw_image.h"

namespace tinygltf
{
	class Node;
	class Model;
}

class RawModel
{
	friend class Model;

private:
	struct Material
	{
		vec4 color = vec4(1);
		float metallic = 1.0f;
		float roughness = 1.0f;
		vec3 emissive = vec3(0);

		int32_t color_texture_index = -1;
		int32_t metallic_roughness_texture_index = -1;
		int32_t normal_texture_index = -1;
		int32_t occlusion_texture_index = -1; 
		int32_t emissive_texture_index = -1;
	};

	struct Primitive
	{
		uint32_t first_index = 0;
		uint32_t index_count = 0;
		uint32_t material_index = 0;
	};

	struct Node
	{
		const Node* parent = nullptr;
		mat4 transform = mat4(1);
		std::vector<Primitive> primitives;
	};

public:
	RawModel(const fs::path& file);

private:
	void loadNode(const tinygltf::Node& node, const Node* parent, const tinygltf::Model& model, std::vector<Vertex3D>& vertex_buffer, std::vector<uint32_t>& index_buffer);

private:
	RawMesh3D mesh;
	std::vector<RawImage<uint8_t>> images;
	std::vector<int32_t> textures;
	std::vector<Material> materials;
	std::vector<Node> nodes;

	fs::path directory;
	fs::path file;
};