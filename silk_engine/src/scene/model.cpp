#include "model.h"
#include "io/file.h"
#include "resources.h"
#include "gfx/graphics.h"

Model::Model(std::string_view file)
{
    *this = load(file);
}

RawModel Model::load(std::string_view file)
{
    return RawModel(file);
}

Model& Model::operator=(const RawModel& raw_model)
{
    meshes = raw_model.meshes;
    path = raw_model.path;

    Image2DProps image_props{};
    auto white = Resources::white_image;
    auto black = Resources::getImage("Black");
    
    images.clear();
    images.reserve(raw_model.material_data.size());
    for (size_t i = 0; i < raw_model.material_data.size(); ++i) //Material data for each mesh
    {
        const auto& md = raw_model.material_data[i];
        images.emplace_back();

        image_props.width = md.diffuse_map.width;
        image_props.height = md.diffuse_map.height;
        image_props.format = Image::getDefaultFormatFromChannelCount(md.diffuse_map.channels);
        image_props.data = md.diffuse_map.data.data();
        images.back().emplace_back((image_props.width != 0 && image_props.height != 0) ? makeShared<Image2D>(image_props) : white);
    
        image_props.width = md.normal_map.width;
        image_props.height = md.normal_map.height;
        image_props.format = Image::getDefaultFormatFromChannelCount(md.normal_map.channels);
        image_props.data = md.normal_map.data.data();
        images.back().emplace_back((image_props.width != 0 && image_props.height != 0) ? makeShared<Image2D>(image_props) : black);
    
        image_props.width = md.height_map.width;
        image_props.height = md.height_map.height;
        image_props.format = Image::getDefaultFormatFromChannelCount(md.height_map.channels);
        image_props.data = md.height_map.data.data();
        images.back().emplace_back((image_props.width != 0 && image_props.height != 0) ? makeShared<Image2D>(image_props) : black);
    
        image_props.width = md.ao_map.width;
        image_props.height = md.ao_map.height;
        image_props.format = Image::getDefaultFormatFromChannelCount(md.ao_map.channels);
        image_props.data = md.ao_map.data.data();
        images.back().emplace_back((image_props.width != 0 && image_props.height != 0) ? makeShared<Image2D>(image_props) : white);
    
        image_props.width = md.specular_map.width;
        image_props.height = md.specular_map.height;
        image_props.format = Image::getDefaultFormatFromChannelCount(md.specular_map.channels);
        image_props.data = md.specular_map.data.data();
        images.back().emplace_back((image_props.width != 0 && image_props.height != 0) ? makeShared<Image2D>(image_props) : black);
    
        image_props.width = md.emissive_map.width;
        image_props.height = md.emissive_map.height;
        image_props.format = Image::getDefaultFormatFromChannelCount(md.emissive_map.channels);
        image_props.data = md.emissive_map.data.data();
        images.back().emplace_back((image_props.width != 0 && image_props.height != 0) ? makeShared<Image2D>(image_props) : black);
    }

    return *this;
}

RawModel::RawModel(std::string_view file)
{
    std::string path = std::string("data/models/") + file.data();
    this->path = path;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
	SK_ASSERT(scene && (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode,
		"Assimp: Couldn't load model at path: {0}", path);

	directory = file.substr(0, file.find_last_of('/'));

    processNode(scene->mRootNode, scene);

    SK_TRACE("Model loaded: {0}", file);
}

void RawModel::processNode(aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
        processMesh(scene->mMeshes[node->mMeshes[i]], scene);

    for (size_t i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

void RawModel::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex3D> vertices(mesh->mNumVertices, Vertex3D{});
    std::vector<uint32_t> indices;

    for(size_t i = 0; i < mesh->mNumVertices; ++i)
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

    auto new_mesh = makeShared<Mesh3D>(vertices, indices);

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Bitmap> diffuse_maps = loadMaterialTextures(material, aiTextureType_DIFFUSE); 
        std::vector<Bitmap> normal_maps = loadMaterialTextures(material, aiTextureType_NORMALS);        
        std::vector<Bitmap> ao_maps = loadMaterialTextures(material, aiTextureType_AMBIENT);
        std::vector<Bitmap> height_maps = loadMaterialTextures(material, aiTextureType_HEIGHT);        
        std::vector<Bitmap> specular_maps = loadMaterialTextures(material, aiTextureType_SPECULAR);        
        std::vector<Bitmap> emissive_maps = loadMaterialTextures(material, aiTextureType_EMISSIVE);

        MeshMaterialData mat{};
        mat.diffuse_map = diffuse_maps.size() ? diffuse_maps[0] : Bitmap{};
        mat.normal_map = normal_maps.size() ? normal_maps[0] : Bitmap{};
        mat.ao_map = ao_maps.size() ? ao_maps[0] : Bitmap{};
        mat.height_map = height_maps.size() ? height_maps[0] : Bitmap{};
        mat.specular_map = specular_maps.size() ? specular_maps[0] : Bitmap{};
        mat.emissive_map = emissive_maps.size() ? emissive_maps[0] : Bitmap{};
        material_data.emplace_back(std::move(mat));
    }
      
    meshes.emplace_back(new_mesh);
}  

std::vector<Bitmap> RawModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
    std::vector<Bitmap> images;

    for (size_t i = 0; i < mat->GetTextureCount(type); ++i)
    {
        if (i > 0) break; //TODO: We don't support multiple textures for each mesh (and we might not)
        aiString str;
        mat->GetTexture(type, i, &str); 

        auto cached_image = image_cache.find(str.C_Str());
        if (cached_image != image_cache.end())
        {
            images.emplace_back(cached_image->second);
        }
        else
        {
            Bitmap image_data = Image2D::load(directory + '/' + str.C_Str());
            if (image_data.channels == 3)
                Image::align4(image_data);
            image_cache.emplace(str.C_Str(), image_data);
            images.emplace_back(std::move(image_data));
        }
    }

    return images;
}