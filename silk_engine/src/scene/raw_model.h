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
		RawImage* diffuse_map;
		RawImage* normal_map;
		RawImage* ao_map;
		RawImage* height_map;
		RawImage* specular_map;
		RawImage* emissive_map;
		static constexpr size_t size() { return 6u; }
	};

public:
	RawModel(std::string_view file);

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<RawImage*> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

private:
	std::vector<shared<RawMesh3D>> meshes;
	std::vector<MeshMaterialData> material_data;
	std::string directory;
	std::string path;

	std::unordered_map<std::string, RawImage> image_cache;
};