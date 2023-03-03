#pragma once

#include "meshes/raw_mesh.h"
#include "gfx/images/raw_image.h"
#include <assimp/Importer.hpp>

class aiNode;
class aiMesh;
class aiMaterial;
enum aiTextureType;

class RawModel
{
	friend class Model;
	struct MeshMaterialData
	{
		RawImage<uint8_t>* diffuse_map;
		RawImage<uint8_t>* normal_map;
		RawImage<uint8_t>* ao_map;
		RawImage<uint8_t>* height_map;
		RawImage<uint8_t>* specular_map;
		RawImage<uint8_t>* emissive_map;
		static constexpr size_t size() { return 6u; }
	};

public:
	RawModel(const path& file);

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<RawImage<uint8_t>*> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

private:
	std::vector<shared<RawMesh3D>> meshes;
	std::vector<MeshMaterialData> material_data;
	path directory;
	path file;

	std::unordered_map<std::string, RawImage<uint8_t>> image_cache;
};