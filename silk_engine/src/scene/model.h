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
	shared<RenderedInstance> processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<shared<Image>> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

private:
	std::vector<shared<RenderedInstance>> meshes; //TODO: Rendered Instance kinda weird? (name: meshes doesn't apply??)
	std::vector<shared<Image>> images;
	std::string directory; 
	std::string path;
	friend class Scene;
};