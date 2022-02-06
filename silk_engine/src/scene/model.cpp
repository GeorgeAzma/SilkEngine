#include "model.h"
#include "io/file.h"
#include "resources.h"

Model::Model(const std::string& file)
{
    std::string path = std::string("data/models/") + file;
    this->path = path;

	Assimp::Importer importer;
    // | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
	
	SK_ASSERT(scene && (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode,
		"Assimp: Couldn't load model at path: {0}", path);

	directory = file.substr(0, file.find_last_of('/'));

    processNode(scene->mRootNode, scene);
    SK_TRACE("Model loaded: {0}", file);
    SK_INFO(meshes.size());
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
       processMesh(scene->mMeshes[node->mMeshes[i]], scene);

    for (size_t i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

void Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices(mesh->mNumVertices, Vertex{});
    std::vector<uint32_t> indices;
    std::vector<shared<Image>> images;

    for(size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices[i].position.x = mesh->mVertices[i].x;
        vertices[i].position.y = mesh->mVertices[i].y;
        vertices[i].position.z = -mesh->mVertices[i].z;

        vertices[i].normal.x = mesh->mNormals[i].x;
        vertices[i].normal.y = mesh->mNormals[i].y;
        vertices[i].normal.z = -mesh->mNormals[i].z;
        
        if (mesh->mTextureCoords[0])
        {
            vertices[i].texture_coordinates.x = mesh->mTextureCoords[0][i].x;
            vertices[i].texture_coordinates.y = mesh->mTextureCoords[0][i].y;
        }
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.emplace_back(mesh->mFaces[i].mIndices[j]);
    }

    shared<Material> new_material = nullptr;
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        std::vector<shared<Image>> diffuse_maps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        images.insert(images.end(), diffuse_maps.begin(), diffuse_maps.end());
        
        std::vector<shared<Image>> specular_maps = loadMaterialTextures(material, aiTextureType_SPECULAR);
        images.insert(images.end(), specular_maps.begin(), specular_maps.end());

        //TODO: Create mesh material
    }
      
    auto new_mesh = makeShared<Mesh>(vertices, indices);
    new_mesh->name = path + std::to_string(meshes.size());

    meshes.emplace_back(new_mesh);
    materials.emplace_back(new_material);
}  

std::vector<shared<Image>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    std::vector<shared<Image>> images;

    for (size_t i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        if (shared<Image> image = Resources::getImage(directory + '/' + str.C_Str()))
        {
            images.emplace_back(image);
        }
        else
        {
            image = makeShared<Image>(directory + '/' + str.C_Str());
            Resources::addImage(image->getPath(), image);
            images.emplace_back(image);
        }
    }

    return images;
}