#pragma once

#include "instance.h"
#include "meshes/mesh.h"
#include "gfx/images/image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model(const std::string& file);

	static void load(const std::string& file);

	const std::vector<shared<Mesh>>& getMeshes() const { return meshes; }
	void setMaterial(size_t index, shared<Material> material) { meshes[index]->material = material; }
	const std::string& getPath() const { return path; }

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<ImageData> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

private:
	std::vector<shared<Mesh>> meshes;
	std::string directory; 
	std::string path;

	std::unordered_map<std::string, ImageData> image_cache;
};