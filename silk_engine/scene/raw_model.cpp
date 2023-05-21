#include "raw_model.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

RawModel::RawModel(const fs::path& file)
{
    fs::path file_path = fs::path("res/models") / file;
    this->file = file_path;
    directory = file.string().substr(0, file.string().find_last_of('/'));

    //const aiScene* scene = importer.ReadFile(file_path.string(), aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);
    tinygltf::Model model;
    std::string err;
    std::string warn;

    static tinygltf::TinyGLTF loader;
    if (file.extension() == ".glb")
        loader.LoadBinaryFromFile(&model, &err, &warn, file_path.string());
    else loader.LoadASCIIFromFile(&model, &err, &warn, file_path.string());

    SK_VERIFY(err.empty(), err);
    SK_VERIFY_WARN(warn.empty(), warn);

    const tinygltf::Scene& scene = model.scenes[0];
    

    // Load images
    images.resize(model.images.size());
    for (size_t i = 0; i < images.size(); ++i)
    {
        const tinygltf::Image& image = model.images[i];
        // We convert RGB-only images to RGBA, as most devices don't support RGB-formats in Vulkan
        RawImage raw_image;
        raw_image.width = image.width;
        raw_image.height = image.height;
        raw_image.channels = image.component;
        if (image.component == 3)
        {
            raw_image.channels = 4;
            raw_image.allocate();
            for (size_t i = 0; i < image.width * image.height; ++i) 
                memcpy(raw_image.pixels.data() + i * 4, image.image.data() + i * 3, sizeof(uint8_t) * 3);
        }
        else raw_image.pixels = std::move(image.image);
        images[i] = std::move(raw_image);
    }

    // Load textures
    textures.resize(model.textures.size());
    for (size_t i = 0; i < model.textures.size(); ++i)
        textures[i] = model.textures[i].source;

    // Load materials
    materials.resize(model.materials.size());
    for (size_t i = 0; i < materials.size(); ++i)
    {
        const tinygltf::Material& material = model.materials[i];
        if (auto it = material.values.find("baseColorFactor"); it != material.values.end())
            materials[i].color = make_vec4(it->second.ColorFactor().data());
        if (auto it = material.values.find("metallicFactor"); it != material.values.end())
            materials[i].metallic = it->second.Factor();
        if (auto it = material.values.find("roughnessFactor"); it != material.values.end())
            materials[i].roughness = it->second.Factor();
        if (auto it = material.values.find("emissiveFactor"); it != material.values.end())
            materials[i].emissive = make_vec3(it->second.ColorFactor().data());

        if (auto it = material.values.find("baseColorTexture"); it != material.values.end())
            materials[i].color_texture_index = it->second.TextureIndex();
        if (auto it = material.values.find("metallicRoughnessTexture"); it != material.values.end())
            materials[i].metallic_roughness_texture_index = it->second.TextureIndex();
        if (auto it = material.values.find("normalTexture"); it != material.values.end())
            materials[i].normal_texture_index = it->second.TextureIndex();
        if (auto it = material.values.find("occlusionTexture"); it != material.values.end())
            materials[i].occlusion_texture_index = it->second.TextureIndex();
    }

    std::vector<Vertex3D> vertex_buffer; 
    std::vector<uint32_t> index_buffer;
    for (int node_idx : scene.nodes)
        loadNode(model.nodes[node_idx], model, vertex_buffer, index_buffer);
    mesh.move(RawMesh3D(vertex_buffer, index_buffer));
  
    SK_TRACE("Model loaded: {}", file);
}

void RawModel::loadNode(const tinygltf::Node& node, const tinygltf::Model& model, std::vector<Vertex3D>& vertex_buffer, std::vector<uint32_t>& index_buffer)
{
    mat4 transform = mat4(1);

    if (node.matrix.size() == 16)
        transform = glm::make_mat4x4(node.matrix.data());
    else
    {
        if (node.translation.size() == 3)
            transform = translate(transform, vec3(make_vec3(node.translation.data())));

        if (node.rotation.size() == 4)
            transform *= mat4(quat(make_quat(node.rotation.data())));

        if (node.scale.size() == 3)
            transform = scale(transform, vec3(make_vec3(node.scale.data())));
    }

    if (node.mesh > -1)
    {
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        for (const tinygltf::Primitive& primitive : mesh.primitives)
        {
            uint32_t vertex_offset = scast<uint32_t>(vertex_buffer.size());

            // Vertices
            {
                const float* positions = nullptr;
                const float* normals = nullptr;
                const float* uvs = nullptr;
                const float* colors = nullptr;
                size_t vertex_count = 0;

                if (auto it = primitive.attributes.find("POSITION"); it != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[it->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    positions = rcast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertex_count = accessor.count;
                }
                if (auto it = primitive.attributes.find("TEXCOORD_0"); it != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[it->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    uvs = rcast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (auto it = primitive.attributes.find("NORMAL"); it != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[it->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    normals = rcast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (auto it = primitive.attributes.find("COLOR_0"); it != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[it->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    colors = rcast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                vertex_buffer.resize(vertex_offset + vertex_count);
                for (size_t v = 0; v < vertex_count; ++v)
                {
                    Vertex3D vertex;
                    vertex.position = make_vec3(positions + v * 3);
                    vertex.uv = uvs ? make_vec2(uvs + v * 2) : vec2(0);
                    vertex.normal = normals ? normalize(make_vec3(normals + v * 3)) : vec3(0);
                    vertex.color = colors ? make_vec4(colors + v * 4) : vec4(1);
                    vertex_buffer[vertex_offset + v] = std::move(vertex);
                }
            }

            // Indices
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[view.buffer];
                uint32_t index_count = scast<uint32_t>(accessor.count);
                uint32_t index_offset = scast<uint32_t>(index_buffer.size());
 
                index_buffer.resize(index_offset + index_count);
                switch (accessor.componentType) 
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: 
                {
                    const uint32_t* buf = rcast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
                    for (size_t i = 0; i < accessor.count; ++i)
                        index_buffer[index_offset + i] = buf[i] + vertex_offset;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: 
                {
                    const uint16_t* buf = rcast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
                    for (size_t i = 0; i < accessor.count; ++i)
                        index_buffer[index_offset + i] = buf[i] + vertex_offset;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: 
                {
                    const uint8_t* buf = rcast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
                    for (size_t i = 0; i < accessor.count; ++i)
                        index_buffer[index_offset + i] = buf[i] + vertex_offset;
                    break;
                }
                default:
                    SK_ERROR("Index component type {} not supported!", accessor.componentType);
                    return;
                }

                primitives.emplace_back(index_offset, index_count, primitive.material);
            }
        }
    }

    for (int child : node.children)
        loadNode(model.nodes[child], model, vertex_buffer, index_buffer);
}