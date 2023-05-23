#include "raw_model.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

// TODO:
// Support multiple texcoords
// Support these: material.alphaCutoff; material.alphaMode; material.doubleSided; primitive.mode;
// Support multiple scenes
// Support different texture map indices, currently they are hardcoded, so only some models will work properly

RawModel::RawModel(const fs::path& file)
    : file(file)
{
    directory = file.string().substr(0, file.string().find_last_of('/'));

    tinygltf::Model model;
    std::string err;
    std::string warn;

    static tinygltf::TinyGLTF loader;
    if (file.extension() == ".glb")
        loader.LoadBinaryFromFile(&model, &err, &warn, file.string());
    else if (file.extension() == ".gltf")
        loader.LoadASCIIFromFile(&model, &err, &warn, file.string());
    else SK_ERROR("Unsupported file extension: {} (supported extensions: gltf, glb)", file.extension());

    SK_VERIFY(err.empty(), err);
    SK_VERIFY_WARN(warn.empty(), warn);

    const tinygltf::Scene& scene = model.scenes[0];
    
    // Load images
    images.resize(model.images.size());
    for (size_t i = 0; i < images.size(); ++i)
    {
        tinygltf::Image& image = model.images[i];
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
            {
                memcpy(raw_image.pixels.data() + i * 4, image.image.data() + i * 3, sizeof(uint8_t) * 3);
                raw_image.pixels[i * 4 + 3] = 255;
            }
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
        materials[i].color = make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());
        materials[i].metallic = material.pbrMetallicRoughness.metallicFactor;
        materials[i].roughness = material.pbrMetallicRoughness.roughnessFactor;
        materials[i].emissive = make_vec3(material.emissiveFactor.data());

        materials[i].color_texture_index = material.pbrMetallicRoughness.baseColorTexture.index;
        materials[i].metallic_roughness_texture_index = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        materials[i].normal_texture_index = material.normalTexture.index;
        materials[i].occlusion_texture_index = material.occlusionTexture.index;
        materials[i].emissive_texture_index = material.emissiveTexture.index;
    }

    std::vector<Vertex3D> vertex_buffer; 
    std::vector<uint32_t> index_buffer;
    nodes.reserve(scene.nodes.size());
    for (int node_idx : scene.nodes)
        loadNode(model.nodes[node_idx], nullptr, model, vertex_buffer, index_buffer);
    mesh.move(RawMesh3D(vertex_buffer, index_buffer));
  
    SK_TRACE("Model loaded: {}", file);
}

void RawModel::loadNode(const tinygltf::Node& node, const Node* parent, const tinygltf::Model& model, std::vector<Vertex3D>& vertex_buffer, std::vector<uint32_t>& index_buffer)
{
    nodes.emplace_back();
    Node& new_node = nodes.back();
    new_node.parent = parent;

    if (node.matrix.size() == 16)
        new_node.transform = glm::make_mat4x4(node.matrix.data());
    else
    {
        if (node.translation.size() == 3)
            new_node.transform = translate(new_node.transform, vec3(make_vec3(node.translation.data())));

        if (node.rotation.size() == 4)
            new_node.transform *= mat4(quat(make_quat(node.rotation.data())));

        if (node.scale.size() == 3)
            new_node.transform = scale(new_node.transform, vec3(make_vec3(node.scale.data())));
    }

    if (parent)
        new_node.transform = parent->transform * new_node.transform;

    for (int child : node.children)
        loadNode(model.nodes[child], &new_node, model, vertex_buffer, index_buffer);
    
    if (node.mesh > -1)
    {
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        new_node.primitives.reserve(mesh.primitives.size());
        for (const tinygltf::Primitive& primitive : mesh.primitives)
        {
            uint32_t vertex_offset = scast<uint32_t>(vertex_buffer.size());
            const float* positions = nullptr;
            const float* normals = nullptr;
            const float* uvs = nullptr;
            const float* colors = nullptr;
            const float* tangents = nullptr;
            size_t vertex_count = 0;
            bool need_tangents = false;

            // Vertices
            {
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
                    need_tangents = true;
                }
                if (auto it = primitive.attributes.find("COLOR_0"); it != primitive.attributes.end())
                {
                    const tinygltf::Accessor& accessor = model.accessors[it->second];
                    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                    colors = rcast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }
                if (need_tangents)
                {
                    if (auto it = primitive.attributes.find("TANGENT"); it != primitive.attributes.end())
                    {
                        const tinygltf::Accessor& accessor = model.accessors[it->second];
                        const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                        tangents = rcast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    }
                }

                vertex_buffer.resize(vertex_offset + vertex_count);
                for (size_t v = 0; v < vertex_count; ++v)
                {
                    Vertex3D vertex{};
                    vertex.position = make_vec3(positions + v * 3);
                    vertex.uv = uvs ? make_vec2(uvs + v * 2) : vec2(0);
                    vertex.uv.y = 1.0f - vertex.uv.y;
                    vertex.normal = normals ? normalize(make_vec3(normals + v * 3)) : vec3(0);
                    vertex.color = colors ? make_vec4(colors + v * 4) : vec4(1);
                    vertex.tangent = tangents ? make_vec4(tangents + v * 4) : vec4(0);
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

                if (!tangents && need_tangents)
                {
                    for (size_t i = index_offset; i < index_offset + index_count; i += 3)
                    {
                        int i0 = index_buffer[i + 0];
                        int i1 = index_buffer[i + 1];
                        int i2 = index_buffer[i + 2];

                        const vec3& p0 = vertex_buffer[i0].position;
                        const vec3& p1 = vertex_buffer[i1].position;
                        const vec3& p2 = vertex_buffer[i2].position;
                        const vec3& n0 = vertex_buffer[i0].normal;
                        const vec3& n1 = vertex_buffer[i1].normal;
                        const vec3& n2 = vertex_buffer[i2].normal;
                        const vec2& uv0 = vertex_buffer[i0].uv;
                        const vec2& uv1 = vertex_buffer[i1].uv;
                        const vec2& uv2 = vertex_buffer[i2].uv;

                        vec3 e1 = p1 - p0;
                        vec3 e2 = p2 - p0;
                        vec2 duv1 = uv1 - uv0;
                        vec2 duv2 = uv2 - uv0;
                        float f = 1.0 / (duv1.x * duv2.y - duv2.x * duv1.y);

                        vec3 t0 = normalize(f * (duv2.y * e1 - duv1.y * e2));
                        vec3 t1 = normalize(f * (duv2.y * e1 - duv1.y * e2));
                        vec3 t2 = normalize(f * (duv2.y * e1 - duv1.y * e2));

                        vec3 cross_product = cross(e1, e2);
                        float handedness = dot(cross_product, n0);

                        vertex_buffer[i0].tangent += vec4(t0, handedness);
                        vertex_buffer[i1].tangent += vec4(t1, handedness);
                        vertex_buffer[i2].tangent += vec4(t2, handedness);
                    }

                    for (size_t i = vertex_offset; i < vertex_offset + vertex_count; ++i)
                        vertex_buffer[i].tangent = vec4(normalize(vec3(vertex_buffer[i].tangent)), sign(vertex_buffer[i].tangent.w));
                }
                
                new_node.primitives.emplace_back(index_offset, index_count, primitive.material);
            }
        }
    }
}