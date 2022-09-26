#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/circle_outline_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "meshes/quad_mesh.h"
#include "meshes/cube_mesh.h"
#include "meshes/sphere_mesh.h"
#include "meshes/triangle_mesh.h"
#include "meshes/tetrahedron_mesh.h"
#include "meshes/rounded_rectangle_mesh.h"
#include "gfx/graphics.h"
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
        if (!std::filesystem::exists("res/cache/shaders"))
            std::filesystem::create_directories("res/cache/shaders");
        if (!std::filesystem::exists("res/images/screenshots"))
            std::filesystem::create_directories("res/images/screenshots");
    }

    //IMAGES
    {       
        Image::Props image_props{};
        image_props.width = 1;
        image_props.height = 1;
        image_props.sampler_props.min_filter = VK_FILTER_NEAREST;
        image_props.sampler_props.mag_filter = VK_FILTER_NEAREST;
        image_props.sampler_props.linear_mipmap = false;
        image_props.sampler_props.anisotropy = 1.0f;
        image_props.mipmap = false;
        constexpr u8vec4 white(255);
        image_props.data = &white;
        white_image = makeShared<Image>(image_props);
        add<Image>("White", white_image);
        
        constexpr u8vec4 black(0);
        image_props.data = &black;
        add<Image>("Black", makeShared<Image>(image_props));
       
        image_props.width = 2;
        image_props.height = 2;
        image_props.sampler_props.u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        image_props.sampler_props.v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        constexpr u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
        image_props.data = null_data;
        add<Image>("Null", makeShared<Image>(image_props));
    }

    //MESHES
    {
        add<Mesh>("Triangle", makeShared<Mesh>(TriangleMesh()));
        add<Mesh>("Circle", makeShared<Mesh>(CircleMesh()));
        add<Mesh>("Circle Outline", makeShared<Mesh>(CircleOutlineMesh()));
        add<Mesh>("Rectangle", makeShared<Mesh>(RectangleMesh()));
        add<Mesh>("Rounded Rectangle", makeShared<Mesh>(RoundedRectangleMesh()));
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
    resources<Image>.clear();
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
template shared<Image> Resources::get<Image>(std::string_view);
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
template void Resources::add<Image>(std::string_view, const shared<Image>&);
template void Resources::add<Font>(std::string_view, const shared<Font>&);

void Resources::addDescriptorSetLayout(const shared<DescriptorSetLayout>& descriptor_layout)
{
    descriptor_set_layouts[DescriptorSetLayoutInfo{descriptor_layout->bindings_vector}] = descriptor_layout;
}

template<typename T>
void Resources::remove(std::string_view name)
{
    resources<T>.erase(name);
}

void Resources::reloadShaders()
{
    for (auto&& [name, pipeline] : resources<GraphicsPipeline>)
        pipeline->recreate();
    for (auto&& [name, pipeline] : resources<ComputePipeline>)
        pipeline->recreate();
}

template void Resources::remove<Mesh>(std::string_view);
template void Resources::remove<Model>(std::string_view);
template void Resources::remove<Shader>(std::string_view);
template void Resources::remove<GraphicsPipeline>(std::string_view);
template void Resources::remove<ComputePipeline>(std::string_view);
template void Resources::remove<Image>(std::string_view);
template void Resources::remove<Font>(std::string_view);

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
    size_t result = bindings.size();
    for (const VkDescriptorSetLayoutBinding& b : bindings)
        result ^= b.binding | (b.descriptorType << 8) | (b.descriptorCount << 16) | (b.stageFlags << 24);

    return result;
}
