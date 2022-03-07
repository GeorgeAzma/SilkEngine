#include "resources.h"
#include "resources.h"
#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "gfx/graphics.h"
#include "gfx/buffers/uniform_buffer.h"
#include "gfx/window/swap_chain.h"
#include "gfx/descriptors/descriptor_set_layout.h"

void Resources::init()
{
    //CREATE NEEDED DIRECTORIES AND INIT STUFF
    {
        if (!std::filesystem::exists("data/cache/shaders"))
            std::filesystem::create_directories("data/cache/shaders");
    }

    //IMAGES
    {
        Image2DProps image_props{};
        image_props.width = 1;
        image_props.height = 1;
        image_props.sampler_props.min_filter = vk::Filter::eNearest;
        image_props.sampler_props.mag_filter = vk::Filter::eNearest;
        image_props.sampler_props.linear_mipmap = false;
        image_props.sampler_props.anisotropy = false;
        image_props.mipmap = false;
        constexpr glm::u8vec4 white(255);
        image_props.data = &white;
        addImage("White", makeShared<Image2D>(image_props));
        constexpr glm::u8vec4 black(0);
        image_props.data = &black;
        addImage("Black", makeShared<Image2D>(image_props));

        image_props = {};
        image_props.width = 2;
        image_props.height = 2;
        image_props.sampler_props.min_filter = vk::Filter::eNearest;
        image_props.sampler_props.mag_filter = vk::Filter::eNearest;
        image_props.sampler_props.linear_mipmap = false;
        image_props.sampler_props.anisotropy = false;
        image_props.sampler_props.u_wrap = vk::SamplerAddressMode::eClampToEdge;
        image_props.sampler_props.v_wrap = vk::SamplerAddressMode::eClampToEdge;
        image_props.mipmap = false;       
        constexpr glm::u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
        image_props.data = null_data;
        addImage("Null", makeShared<Image2D>(image_props));

        addImage("Test1", makeShared<Image2D>("test1.png"));
        addImage("Test2", makeShared<Image2D>("test2.png"));
    }

    //MESHES
    {
        addMesh2D("Circle", makeShared<CircleMesh>());
        addMesh2D("Rectangle", makeShared<RectangleMesh>());
        addMesh("Circle", *getMesh2D("Circle"));
        addMesh("Rectangle", *getMesh2D("Rectangle"));
    }

    //MODELS
    {
        addModel("Backpack", makeShared<Model>("backpack/backpack.obj"));
    }

    //FONTS
    {
        addFont("Arial", makeShared<Font>("arial.ttf"));
    }

    //MATERIALS
    {
        auto shader = makeShared<Shader>("3D");
        shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .setShader(shader)
            .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
            .setSampleCount(Graphics::swap_chain->getSampleCount())
            .setRenderPass(Graphics::swap_chain->getRenderPass())
            .build();
        addShaderEffect("Lit 3D", makeShared<ShaderEffect>(graphics_pipeline));

        vk::Bool32 lit = false;
        graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .setShader(shader, { { "lit", &lit, sizeof(vk::Bool32) }})
            .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
            .setSampleCount(Graphics::swap_chain->getSampleCount())
            .setRenderPass(Graphics::swap_chain->getRenderPass())
            .build();
        addShaderEffect("3D", makeShared<ShaderEffect>(graphics_pipeline));

        shared<ComputePipeline> compute_pipeline = makeShared<ComputePipeline>(makeShared<Shader>("bgra_to_rgba"));
        addComputeShaderEffect("BGRA To RGBA", makeShared<ComputeShaderEffect>(compute_pipeline)); 
    }
}

void Resources::cleanup()
{
    fonts.clear();
	meshes.clear();
    meshes2D.clear();
	models.clear();
    shader_effects.clear();
    compute_shader_effects.clear();
    images.clear();
    descriptor_set_layouts.clear();
}

shared<Mesh> Resources::getMesh(const std::string& name)
{
    auto it = meshes.find(name);
    return it == meshes.end() ? nullptr : it->second;
}

shared<Mesh2D> Resources::getMesh2D(const std::string& name)
{
    auto it = meshes2D.find(name);
    return it == meshes2D.end() ? nullptr : it->second;
}

shared<Model> Resources::getModel(const std::string& name)
{
    auto it = models.find(name);
    return it == models.end() ? nullptr : it->second;
}

shared<ShaderEffect> Resources::getShaderEffect(const std::string& name)
{
    auto it = shader_effects.find(name);
    return it == shader_effects.end() ? nullptr : it->second;
}

shared<ComputeShaderEffect> Resources::getComputeShaderEffect(const std::string& name)
{
    auto it = compute_shader_effects.find(name);
    return it == compute_shader_effects.end() ? nullptr : it->second;
}

shared<Image2D> Resources::getImage(const std::string& name)
{
    auto it = images.find(name);
    return it == images.end() ? nullptr : it->second;
}

shared<DescriptorSetLayout> Resources::getDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
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
    auto it = fonts.find(name);
    return it == fonts.end() ? nullptr : it->second;
}

void Resources::addMesh(const std::string& name, shared<Mesh> mesh)
{
	meshes[name] = mesh;
}

void Resources::addMesh2D(const std::string& name, shared<Mesh2D> mesh)
{
    meshes2D[name] = mesh;
}

void Resources::addModel(const std::string& name, shared<Model> model)
{
    models[name] = model;
}

void Resources::addShaderEffect(const std::string& name, shared<ShaderEffect> shader_effect)
{
    shader_effects[name] = shader_effect;
}

void Resources::addComputeShaderEffect(const std::string& name, shared<ComputeShaderEffect> compute_shader_effect)
{
    compute_shader_effects[name] = compute_shader_effect;
}

void Resources::addImage(const std::string& name, shared<Image2D> image)
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

    for (const vk::DescriptorSetLayoutBinding& b : bindings)
    {
        size_t binding_hash = b.binding | (VkDescriptorType(b.descriptorType) << 8) | (uint32_t(b.descriptorCount) << 16) | (VkShaderStageFlags(b.stageFlags) << 24);
        result ^= std::hash<size_t>()(binding_hash);
    }

    return result;
}
