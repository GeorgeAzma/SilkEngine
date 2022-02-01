#include "model.h"
#include "io/file.h"

Model::Model(const std::string& file)
{
    std::string path = std::string("data/models/") + file;
    this->path = path;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);
	
	SK_ASSERT(scene && (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode,
		"Assimp: Couldn't load model at path: {0}", path);

	directory = file.substr(0, file.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        meshes.emplace_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
        if(meshes.back()->mesh->name == "") 
            meshes.back()->mesh->name = path + std::to_string(meshes.size());
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

shared<RenderedInstance> Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices(mesh->mNumVertices, Vertex{});
    std::vector<uint32_t> indices;
    std::vector<shared<Image>> images;

    for(size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices[i].position.x = mesh->mVertices[i].x;
        vertices[i].position.y = mesh->mVertices[i].y;
        vertices[i].position.z = mesh->mVertices[i].z;

        vertices[i].normal.x = mesh->mNormals[i].x;
        vertices[i].normal.y = mesh->mNormals[i].y;
        vertices[i].normal.z = mesh->mNormals[i].z;
        
        if (mesh->mTextureCoords[0])
        {
            vertices[i].texture_coordinates.x = mesh->mTextureCoords[0][i].x;
            vertices[i].texture_coordinates.y = mesh->mTextureCoords[0][i].y;
        }
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        //uint32_t* last = &indices.back();
        //indices.resize(indices.size() + mesh->mFaces[i].mNumIndices);
        //std::memcpy(last, mesh->mFaces[i].mIndices, mesh->mFaces[i].mNumIndices * sizeof(uint32_t));
        for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.emplace_back(mesh->mFaces[i].mIndices[j]);
    }

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        std::vector<shared<Image>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        images.insert(images.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        std::vector<shared<Image>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
        images.insert(images.end(), specularMaps.begin(), specularMaps.end());
    }

    return makeShared<RenderedInstance>(makeShared<Mesh>(vertices, indices/*, textures*/));
}  

std::vector<shared<Image>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    std::vector<shared<Image>> images;

    for (size_t i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (size_t j = 0; j < images_loaded.size(); ++j)
        {
            if (std::strcmp(images_loaded[j]->getPath().data(), (directory + '/' + str.C_Str()).c_str()) == 0)
            {
                images.emplace_back(images_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            shared<Image> image = makeShared<Image>(directory + '/' + str.C_Str());
            images.emplace_back(image);
            images_loaded.emplace_back(image);
        }
    }

    return images;
}