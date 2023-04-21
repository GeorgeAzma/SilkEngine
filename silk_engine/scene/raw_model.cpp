#include "raw_model.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

static tinygltf::TinyGLTF loader;

RawModel::RawModel(const path& file)
{
    path file_path = path("res/models") / file;
    this->file = file_path;

    //Assimp::Importer importer;
    //const aiScene* scene = importer.ReadFile(file_path.string(), aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
    //SK_VERIFY(scene && (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode, "Assimp: Couldn't load model at path: {}", file_path);

    directory = file.string().substr(0, file.string().find_last_of('/'));

    tinygltf::Model model;
    std::string err;
    std::string warn;


    if (file.extension() == ".glb")
        loader.LoadBinaryFromFile(&model, &err, &warn, file_path.string());
    else loader.LoadASCIIFromFile(&model, &err, &warn, file_path.string());

    SK_VERIFY(err.empty(), err);
    SK_VERIFY_WARN(warn.empty(), warn);

    SK_CRITICAL("Unfinished code, can't load a model");

    SK_TRACE("Model loaded: {}", file);
}
/*
void RawModel::processNode(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
        processMesh(scene->mMeshes[node->mMeshes[i]], scene);

    for (size_t i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

void RawModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex3D> vertices(mesh->mNumVertices, Vertex3D{});
    std::vector<uint32_t> indices;

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        if (mesh->HasPositions())
        {
            vertices[i].position.x = mesh->mVertices[i].x;
            vertices[i].position.y = mesh->mVertices[i].y;
            vertices[i].position.z = -mesh->mVertices[i].z;
        }

        if (mesh->HasNormals())
        {
            vertices[i].normal.x = mesh->mNormals[i].x;
            vertices[i].normal.y = mesh->mNormals[i].y;
            vertices[i].normal.z = -mesh->mNormals[i].z;
        }

        if (mesh->HasVertexColors(0))
        {
            vertices[i].color.r = mesh->mColors[0][i].r;
            vertices[i].color.g = mesh->mColors[0][i].g;
            vertices[i].color.b = mesh->mColors[0][i].b;
            vertices[i].color.a = mesh->mColors[0][i].a;
        }

        if (mesh->HasTextureCoords(0))
        {
            vertices[i].texture_coordinate.x = mesh->mTextureCoords[0][i].x;
            vertices[i].texture_coordinate.y = mesh->mTextureCoords[0][i].y;
        }
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        for (size_t j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
            indices.emplace_back(mesh->mFaces[i].mIndices[j]);
    }

    shared<RawMesh3D> new_mesh = makeShared<RawMesh3D>(vertices, indices);

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<RawImage<uint8_t>*> diffuse_maps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
        std::vector<RawImage<uint8_t>*> normal_maps = loadMaterialTextures(material, aiTextureType_NORMALS);
        std::vector<RawImage<uint8_t>*> ao_maps = loadMaterialTextures(material, aiTextureType_AMBIENT);
        std::vector<RawImage<uint8_t>*> height_maps = loadMaterialTextures(material, aiTextureType_HEIGHT);
        std::vector<RawImage<uint8_t>*> specular_maps = loadMaterialTextures(material, aiTextureType_SPECULAR);
        std::vector<RawImage<uint8_t>*> emissive_maps = loadMaterialTextures(material, aiTextureType_EMISSIVE);

        MeshMaterialData mat{};
        mat.diffuse_map = diffuse_maps.size() ? diffuse_maps[0] : nullptr;
        mat.normal_map = normal_maps.size() ? normal_maps[0] : nullptr;
        mat.ao_map = ao_maps.size() ? ao_maps[0] : nullptr;
        mat.height_map = height_maps.size() ? height_maps[0] : nullptr;
        mat.specular_map = specular_maps.size() ? specular_maps[0] : nullptr;
        mat.emissive_map = emissive_maps.size() ? emissive_maps[0] : nullptr;
        material_data.emplace_back(std::move(mat));
    }

    meshes.emplace_back(new_mesh);
}

std::vector<RawImage<uint8_t>*> RawModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    std::vector<RawImage<uint8_t>*> images;

    for (size_t i = 0; i < mat->GetTextureCount(type); ++i)
    {
        if (i > 0) break; //TODO: We don't support multiple textures for each mesh (and we might not)
        aiString str;
        mat->GetTexture(type, i, &str);

        auto cached_image = image_cache.find(str.C_Str());
        if (cached_image != image_cache.end())
        {
            images.emplace_back(&cached_image->second);
        }
        else
        {
            RawImage image_data{};
            image_data.load(directory / str.C_Str());
            if (image_data.channels == 3)
                image_data.align4();
            image_cache.emplace(str.C_Str(), std::move(image_data));
            images.emplace_back(&image_cache.at(str.C_Str()));
        }
    }

    return images;
}
*/