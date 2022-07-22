#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "meshes/quad_mesh.h"
#include "meshes/cube_mesh.h"
#include "meshes/sphere_mesh.h"
#include "meshes/triangle_mesh.h"
#include "meshes/tetrahedron_mesh.h"
#include "gfx/graphics.h"
#include "gfx/buffers/uniform_buffer.h"
#include "gfx/window/swap_chain.h"
#include "gfx/descriptors/descriptor_set_layout.h"
#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/allocators/command_pool.h"
#include "utils/thread_pool.h"
#include "model.h"
#include "gfx/ui/font.h"

ThreadPool Resources::pool;

void Resources::init()
{
    //CREATE NEEDED DIRECTORIES AND INIT STUFF
    {
        if (!std::filesystem::exists("data/cache/shaders"))
            std::filesystem::create_directories("data/cache/shaders");
        if (!std::filesystem::exists("data/images/screenshots"))
            std::filesystem::create_directories("data/images/screenshots");
    }

    //IMAGES
    {       
        Image2DProps image_props{};
        image_props.width = 1;
        image_props.height = 1;
        image_props.sampler_props.min_filter = VK_FILTER_NEAREST;
        image_props.sampler_props.mag_filter = VK_FILTER_NEAREST;
        image_props.sampler_props.linear_mipmap = false;
        image_props.sampler_props.anisotropy = false;
        image_props.mipmap = false;
        constexpr glm::u8vec4 white(255);
        image_props.data = &white;
        white_image = makeShared<Image2D>(image_props);
        add<Image2D>("White", white_image);
        
        constexpr glm::u8vec4 black(0);
        image_props.data = &black;
        add<Image2D>("Black", makeShared<Image2D>(image_props));
       
        image_props.width = 2;
        image_props.height = 2;
        image_props.sampler_props.u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        image_props.sampler_props.v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        constexpr glm::u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
        image_props.data = null_data;
        add<Image2D>("Null", makeShared<Image2D>(image_props));
    }

    //MESHES
    {
        add<Mesh>("Triangle", makeShared<Mesh>(TriangleMesh()));
        add<Mesh>("Circle", makeShared<Mesh>(CircleMesh()));
        add<Mesh>("Rectangle", makeShared<Mesh>(RectangleMesh()));
        add<Mesh>("Quad", makeShared<Mesh>(QuadMesh()));
        add<Mesh>("Cube", makeShared<Mesh>(CubeMesh()));
        add<Mesh>("Sphere", makeShared<Mesh>(SphereMesh()));
        add<Mesh>("Tetrahedron", makeShared<Mesh>(TetrahedronMesh()));
    }

    //MODELS
    {
    }

    //FONTS
    {
        add<Font>("Arial", makeShared<Font>("arial.ttf"));
    }

    //SHADERS
    {
        add<Shader>("3D", makeShared<Shader>("3D"));
        add<Shader>("2D", makeShared<Shader>("2D"));
        add<Shader>("Font", makeShared<Shader>("font"));
        add<Shader>("BGRA To RGBA", makeShared<Shader>("bgra_to_rgba"));
    }

    //PIPELINES
    {
        add<ComputePipeline>("BGRA To RGBA", makeShared<ComputePipeline>(get<Shader>("BGRA To RGBA")));
    }
}
 
void Resources::destroy()
{
    resources<Mesh>.clear();
    resources<Model>.clear();
    resources<Shader>.clear();
    resources<GraphicsPipeline>.clear();
    resources<ComputePipeline>.clear();
    resources<Image2D>.clear();
    resources<Font>.clear();
    white_image = nullptr;
    descriptor_set_layouts.clear();
}

template<typename T>
shared<T> Resources::get(std::string_view name)
{
    return resources<T>.at(name);
}
template shared<Mesh> Resources::get<Mesh>(std::string_view);
template shared<Model> Resources::get<Model>(std::string_view);
template shared<Shader> Resources::get<Shader>(std::string_view);
template shared<GraphicsPipeline> Resources::get<GraphicsPipeline>(std::string_view);
template shared<ComputePipeline> Resources::get<ComputePipeline>(std::string_view);
template shared<Image2D> Resources::get<Image2D>(std::string_view);
template shared<Font> Resources::get<Font>(std::string_view);

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

template<typename T>
void Resources::add(std::string_view name, const shared<T>& t)
{
    resources<T>.insert_or_assign(name, t);
}
template void Resources::add<Mesh>(std::string_view, const shared<Mesh>&);
template void Resources::add<Model>(std::string_view, const shared<Model>&);
template void Resources::add<Shader>(std::string_view, const shared<Shader>&);
template void Resources::add<GraphicsPipeline>(std::string_view, const shared<GraphicsPipeline>&);
template void Resources::add<ComputePipeline>(std::string_view, const shared<ComputePipeline>&);
template void Resources::add<Image2D>(std::string_view, const shared<Image2D>&);
template void Resources::add<Font>(std::string_view, const shared<Font>&);

void Resources::addDescriptorSetLayout(const shared<DescriptorSetLayout>& descriptor_layout)
{
    descriptor_set_layouts[DescriptorSetLayoutInfo{descriptor_layout->bindings_vector}] = descriptor_layout;
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
        size_t binding_hash = b.binding | (VkDescriptorType(b.descriptorType) << 8) | (uint32_t(b.descriptorCount) << 16) | (VkShaderStageFlags(b.stageFlags) << 24);
        result ^= std::hash<size_t>()(binding_hash);
    }

    return result;
}
