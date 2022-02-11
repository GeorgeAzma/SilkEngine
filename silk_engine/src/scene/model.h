#pragma once

#include "instance.h"
#include "meshes/mesh.h"
#include "gfx/images/image2D.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class RawModel
{
	friend class Model;
	struct MeshMaterialData
	{
		ImageData diffuse_map;
		ImageData normal_map;
		ImageData ao_map;
		ImageData height_map;
		ImageData specular_map;
		ImageData emissive_map;
		static constexpr size_t size() { return 6u; }
	};
public:
	RawModel(const std::string& file);

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<ImageData> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

private:
	std::vector<shared<Mesh>> meshes;
	std::vector<MeshMaterialData> material_data;
	std::string directory;
	std::string path;

	std::unordered_map<std::string, ImageData> image_cache;
};

class Model
{
public:
	Model(const std::string& file);

	static RawModel load(const std::string& file);

	void setMaterial(size_t index, shared<Material> material) { meshes[index]->material = material; }

	const std::vector<shared<Mesh>>& getMeshes() const { return meshes; }
	const std::string& getPath() const { return path; }

	Model& operator=(const RawModel & raw_model);

private:
	std::vector<shared<Mesh>> meshes;
	std::vector<shared<Image2D>> images;
	std::string path;
};