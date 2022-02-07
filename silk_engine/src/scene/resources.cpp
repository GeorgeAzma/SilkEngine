#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "gfx/graphics.h"
#include "gfx/buffers/uniform_buffer.h"
#include "gfx/window/swap_chain.h"
#include "gfx/descriptors/descriptor_set_layout.h"

void Resources::init()
{
    //CREATE NEEDED DIRECTORIES
    {
        if (!std::filesystem::exists("data/cache/shaders"))
            std::filesystem::create_directories("data/cache/shaders");
    }

    //MESHES
    {
        addMesh("Circle", makeShared<CircleMesh>());
        addMesh("Rectangle", makeShared<RectangleMesh>());
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
        shared<DescriptorSet> global_descriptor_set = makeShared<DescriptorSet>();
        global_descriptor_set->addBuffers(0, { { *Graphics::global_uniform, 0, VK_WHOLE_SIZE } }, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
        addDescriptorSet("Global", global_descriptor_set);

        auto white_image = Resources::getImage("White");
        shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>();
        descriptor_set->addImages(0, {
                *Resources::getImage("Test1"), *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image,
                *white_image, *white_image, *white_image, *white_image
            }, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        addDescriptorSet("Images", descriptor_set);

        shared<Shader> shader = makeShared<Shader>("3D");
        shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .addDescriptorSetLayout(global_descriptor_set->getLayout())
            .addDescriptorSetLayout(descriptor_set->getLayout())
            .setShader(shader)
            .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
            .setSampleCount(Graphics::swap_chain->getSampleCount())
            .setRenderPass(Graphics::swap_chain->getRenderPass())
            .build();
        addShaderEffect("3D", makeShared<ShaderEffect>(graphics_pipeline));
    }

    //MODELS
    {
        addModel("Backpack", makeShared<Model>("backpack/backpack.obj"));
    }
}

void Resources::cleanup()
{
    fonts.clear();
	meshes.clear();
	models.clear();
    shader_effects.clear();
    compute_shader_effects.clear();
    images.clear();
    descriptor_set_layouts.clear();
    descriptor_sets.clear();
}

shared<Mesh> Resources::getMesh(const std::string& name)
{
    auto it = meshes.find(name);
    return it == meshes.end() ? nullptr : it->second;
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

shared<Image> Resources::getImage(const std::string& name)
{
    auto it = images.find(name);
    return it == images.end() ? nullptr : it->second;
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

shared<DescriptorSet> Resources::getDescriptorSet(const std::string& name)
{
    auto it = descriptor_sets.find(name);
    return it == descriptor_sets.end() ? nullptr : it->second;
}

shared<Font> Resources::getFont(const std::string& name)
{
    auto it = fonts.find(name);
    return it == fonts.end() ? nullptr : it->second;
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

void Resources::addShaderEffect(const std::string& name, shared<ShaderEffect> shader_effect)
{
    shader_effects[name] = shader_effect;
}

void Resources::addComputeShaderEffect(const std::string& name, shared<ComputeShaderEffect> compute_shader_effect)
{
    compute_shader_effects[name] = compute_shader_effect;
}

void Resources::addImage(const std::string& name, shared<Image> image)
{
    images[name] = image;
}

void Resources::addDescriptorSetLayout(shared<DescriptorSetLayout> descriptor_layout)
{
    descriptor_set_layouts[DescriptorSetLayoutInfo{descriptor_layout->bindings_vector}] = descriptor_layout;
}

void Resources::addDescriptorSet(const std::string& name, shared<DescriptorSet> descriptor_set)
{
    descriptor_sets[name] = descriptor_set;
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
