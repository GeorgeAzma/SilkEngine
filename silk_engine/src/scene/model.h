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

private:
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<shared<Image>> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

private:
	std::vector<shared<Mesh>> meshes;
	std::vector<shared<Material>> materials;
	std::string directory; 
	std::string path;
	friend class Scene;
};