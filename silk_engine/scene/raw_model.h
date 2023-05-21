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
		float metallic = 0.5f;
		float roughness = 0.5f;
		vec3 emissive = vec3(0);

		uint32_t color_texture_index = 0;
		uint32_t metallic_roughness_texture_index = 0;
		uint32_t normal_texture_index = 0;
		uint32_t occlusion_texture_index = 0;

		constexpr size_t getTextureCount() const { return 4; }
	};

	struct Primitive
	{
		uint32_t first_index = 0;
		uint32_t index_count = 0;
		uint32_t material_index = 0;
	};

public:
	RawModel(const fs::path& file);

private:
	void loadNode(const tinygltf::Node& node, const tinygltf::Model& model, std::vector<Vertex3D>& vertex_buffer, std::vector<uint32_t>& index_buffer);

private:
	RawMesh3D mesh;
	std::vector<RawImage<uint8_t>> images;
	std::vector<uint32_t> textures;
	std::vector<Material> materials;
	std::vector<Primitive> primitives;

	fs::path directory;
	fs::path file;
};