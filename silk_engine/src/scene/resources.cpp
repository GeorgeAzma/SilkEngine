#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "gfx/graphics.h"
#include "gfx/buffers/uniform_buffer.h"
#include "gfx/window/swap_chain.h"
#include "gfx/descriptors/descriptor_set_layout.h"

void Resources::init()
{
    //MESHES
    {
        addMesh("Circle", makeShared<CircleMesh>());
        addMesh("Rectangle", makeShared<RectangleMesh>());
    }

    //MODELS
    {
        addModel("Backpack", makeShared<Model>("backpack/backpack.obj"));
    }

    //IMAGES
    {
        ImageProps white_image_props{};
        white_image_props.width = 1;
        white_image_props.height = 1;
        white_image_props.sampler_props.min_filter = VK_FILTER_NEAREST;
        white_image_props.sampler_props.mag_filter = VK_FILTER_NEAREST;
        white_image_props.sampler_props.linear_mipmap = false;
        white_image_props.sampler_props.anisotropy = false;
        white_image_props.mipmap = false;
        constexpr glm::u8vec4 white(255);
        white_image_props.data = &white;
        shared<Image> white_image = makeShared<Image>(white_image_props);
        addImage("White", white_image);

        ImageProps null_image_props{};
        null_image_props.width = 2;
        null_image_props.height = 2;
        null_image_props.sampler_props.min_filter = VK_FILTER_NEAREST;
        null_image_props.sampler_props.mag_filter = VK_FILTER_NEAREST;
        null_image_props.sampler_props.linear_mipmap = false;
        null_image_props.sampler_props.anisotropy = false;
        null_image_props.sampler_props.u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        null_image_props.sampler_props.v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        null_image_props.mipmap = false;
        constexpr glm::u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
        null_image_props.data = null_data;
        shared<Image> null_image = makeShared<Image>(null_image_props);
        addImage("Null", null_image);

        shared<Image> test1_image = makeShared<Image>("test1.png");
        shared<Image> test2_image = makeShared<Image>("test2.png");
        addImage("Test1", test1_image);
        addImage("Test2", test2_image);
    }

    //FONTS
    {
        addFont("Arial", makeShared<Font>("arial.ttf"));
    }

    //MATERIALS
    {
        shared<DescriptorSetLayout> global_descriptor_set_layout = makeShared<DescriptorSetLayout>();
        global_descriptor_set_layout->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .build();
        addDescriptorSetLayout(global_descriptor_set_layout);

        shared<DescriptorSetLayout> descriptor_set_layout = makeShared<DescriptorSetLayout>();
        descriptor_set_layout->addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, Graphics::MAX_TEXTURE_SLOTS)
            .build();
        addDescriptorSetLayout(descriptor_set_layout);

        shared<Shader> shader = makeShared<Shader>(std::vector<std::string>{"3D.vert", "3D.frag"});
        shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .addDescriptorSetLayout(global_descriptor_set_layout)
            .addDescriptorSetLayout(descriptor_set_layout)
            .setShader(shader)
            .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
            .setSampleCount(Graphics::swap_chain->getSampleCount())
            .setRenderPass(Graphics::swap_chain->getRenderPass())
            .build();
        addMaterial("3D", makeShared<Material>(graphics_pipeline));
        
        shader = makeShared<Shader>(std::vector<std::string>{"batch3D.vert", "batch3D.frag"});
        graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .addDescriptorSetLayout(global_descriptor_set_layout)
            .addDescriptorSetLayout(descriptor_set_layout)
            .setShader(shader)
            .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::UINT }, { Type::VEC4 } })
            .setSampleCount(Graphics::swap_chain->getSampleCount())
            .setRenderPass(Graphics::swap_chain->getRenderPass())
            .build();
        addMaterial("Batch3D", makeShared<Material>(graphics_pipeline));
    }
    
    //COMPUTE MATERIALS

}

void Resources::cleanup()
{
    fonts.clear();
	meshes.clear();
	models.clear();
	materials.clear();
	compute_materials.clear();
    images.clear();
    descriptor_set_layouts.clear();
}

shared<Mesh> Resources::getMesh(const std::string& name)
{
	return meshes.at(name);
}

shared<Model> Resources::getModel(const std::string& name)
{
    return models.at(name);
}

shared<Material> Resources::getMaterial(const std::string& name)
{
    return materials.at(name);
}

shared<ComputeMaterial> Resources::getComputeMaterial(const std::string& name)
{
    return compute_materials.at(name);
}

shared<Image> Resources::getImage(const std::string& name)
{
    return images.at(name);
}

shared<DescriptorSetLayout> Resources::getDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    auto layout = descriptor_set_layouts.find({ bindings });
    if (layout != descriptor_set_layouts.end())
        return layout->second;

    shared<DescriptorSetLayout> new_layout = makeShared<DescriptorSetLayout>();
    for (size_t i = 0; i < bindings.size(); ++i)
        new_layout->addBinding(bindings[i].binding, bindings[i].descriptorType, bindings[i].stageFlags, bindings[i].descriptorCount);
    new_layout->build();
    addDescriptorSetLayout(new_layout);

    return new_layout;
}

shared<Font> Resources::getFont(const std::string& name)
{
    return fonts.at(name);
}

void Resources::addMesh(const std::string& name, shared<Mesh> mesh)
{
	mesh->name = name;
	meshes[name] = mesh;
}

void Resources::addModel(const std::string& name, shared<Model> model)
{
    models[name] = model;
}

void Resources::addMaterial(const std::string& name, shared<Material> material)
{
    material->name = name;
	materials[name] = material;
}

void Resources::addComputeMaterial(const std::string& name, shared<ComputeMaterial> compute_material)
{
    compute_material->name = name;
    compute_materials[name] = compute_material;
}

void Resources::addImage(const std::string& name, shared<Image> image)
{
    images[name] = image;
}

void Resources::addDescriptorSetLayout(shared<DescriptorSetLayout> descriptor_layout)
{
    descriptor_set_layouts[DescriptorSetLayoutInfo{descriptor_layout->bindings_vector}] = descriptor_layout;
}

void Resources::addFont(const std::string& name, shared<Font> font)
{
    fonts[name] = font;
}

bool Resources::DescriptorSetLayoutInfo::operator==(const DescriptorSetLayoutInfo& other) const
{
    if (other.bindings.size() != bindings.size())
        return false;

   for (size_t i = 0; i < bindings.size(); i++) 
   {
       if (other.bindings[i].binding != bindings[i].binding ||
           other.bindings[i].descriptorType != bindings[i].descriptorType ||
           other.bindings[i].descriptorCount != bindings[i].descriptorCount ||
           other.bindings[i].stageFlags != bindings[i].stageFlags)
           return false;
   }

   return true;
}

size_t Resources::DescriptorSetLayoutInfo::hash() const
{
    size_t result = std::hash<size_t>()(bindings.size());

    for (const VkDescriptorSetLayoutBinding& b : bindings)
    {
        size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;
        result ^= std::hash<size_t>()(binding_hash);
    }

    return result;
}
