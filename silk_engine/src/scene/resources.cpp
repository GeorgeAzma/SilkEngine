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
        addImage("White", white_image);
        
        constexpr glm::u8vec4 black(0);
        image_props.data = &black;
        addImage("Black", makeShared<Image2D>(image_props));
       
        image_props.width = 2;
        image_props.height = 2;
        image_props.sampler_props.u_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        image_props.sampler_props.v_wrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        constexpr glm::u8vec4 null_data[4] = { { 0, 0, 0, 255 }, { 255, 0, 255, 255 }, { 255, 0, 255, 255 }, { 0, 0, 0, 255 } };
        image_props.data = null_data;
        addImage("Null", makeShared<Image2D>(image_props));
    }

    //MESHES
    {
        addMesh("Triangle", makeShared<TriangleMesh>());
        addMesh("Circle", makeShared<CircleMesh>());
        addMesh("Rectangle", makeShared<RectangleMesh>());
        addMesh("Quad", makeShared<QuadMesh>());
        addMesh("Circle3D", shared<Mesh3D>(*(Mesh2D*)(getMesh("Circle").get())));
        addMesh("Rectangle3D", shared<Mesh3D>(*(Mesh2D*)(getMesh("Rectangle").get())));
        addMesh("Cube", makeShared<CubeMesh>());
        addMesh("Sphere", makeShared<SphereMesh>());
        addMesh("Tetrahedron", makeShared<TetrahedronMesh>());
    }

    //MODELS
    {
    }

    //FONTS
    {
        addFont("Arial", makeShared<Font>("arial.ttf"));
    }

    //SHADERS
    {
        addShader("3D", makeShared<Shader>("3D"));
        addShader("2D", makeShared<Shader>("2D"));
        addShader("Font", makeShared<Shader>("font"));
        addShader("BGRA To RGBA", makeShared<Shader>("bgra_to_rgba"));
    }

    //PIPELINES
    {       
        using enum DeviceType;
        shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .setShader(getShader("3D"))
            .setVertexLayout({ { VEC3 }, { VEC2 }, { VEC3 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
            .setSamples(Graphics::swap_chain->getSamples())
            .setRenderPass(*Graphics::swap_chain->getRenderPass())
            .build();
        addGraphicsPipeline("Lit 3D", graphics_pipeline);

        VkBool32 lit = false;
        graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .setShader(getShader("3D"), { { "lit", &lit, sizeof(VkBool32) } })
            .setVertexLayout({ { VEC3 }, { VEC2 }, { VEC3 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
            .setSamples(Graphics::swap_chain->getSamples())
            .setRenderPass(*Graphics::swap_chain->getRenderPass())
            .build();
        addGraphicsPipeline("3D", graphics_pipeline);
        
        graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .setDepthCompareOp(VK_COMPARE_OP_ALWAYS)
            .setShader(getShader("2D"))
            .setVertexLayout({ { VEC2 }, { VEC2 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
            .setSamples(Graphics::swap_chain->getSamples())
            .setRenderPass(*Graphics::swap_chain->getRenderPass())
            .build();
        addGraphicsPipeline("2D", graphics_pipeline);

        graphics_pipeline = makeShared<GraphicsPipeline>();
        graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
            .enable(EnableTag::DEPTH_TEST)
            .enable(EnableTag::DEPTH_WRITE)
            .setDepthCompareOp(VK_COMPARE_OP_ALWAYS)
            .setShader(getShader("Font"))
            .setVertexLayout({ { VEC2 }, { VEC2 }, { VEC4 }, { MAT4, 1 }, { UINT, 1 }, { VEC4, 1 } })
            .setSamples(Graphics::swap_chain->getSamples())
            .setRenderPass(*Graphics::swap_chain->getRenderPass())
            .build();
        addGraphicsPipeline("Font", graphics_pipeline);

        addComputePipeline("BGRA To RGBA", makeShared<ComputePipeline>(getShader("BGRA To RGBA")));
    }
}
 
void Resources::cleanup()
{
	meshes.clear();
	models.clear();
    fonts.clear();
    shaders.clear();
    graphics_pipelines.clear();
    compute_pipelines.clear();
    white_image = nullptr;
    images.clear();
    descriptor_set_layouts.clear();
}

shared<Mesh> Resources::getMesh(std::string_view name)
{
    return meshes.at(name);
}

shared<Model> Resources::getModel(std::string_view name)
{
    return models.at(name);
}

shared<Shader> Resources::getShader(std::string_view name)
{
    return shaders.at(name);
}

shared<GraphicsPipeline> Resources::getGraphicsPipeline(std::string_view name)
{
    return graphics_pipelines.at(name);
}

shared<ComputePipeline> Resources::getComputePipeline(std::string_view name)
{
    return compute_pipelines.at(name);
}

shared<Image2D> Resources::getImage(std::string_view name)
{
    return images.at(name);
}

shared<Font> Resources::getFont(std::string_view name)
{
    return fonts.at(name);
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

void Resources::addMesh(std::string_view name, const shared<Mesh>& mesh)
{
    meshes.insert_or_assign(name, mesh);
}

void Resources::addModel(std::string_view name, const shared<Model>& model)
{
    models.insert_or_assign(name, model);
}

void Resources::addShader(std::string_view name, const shared<Shader>& shader)
{
    shaders.insert_or_assign(name, shader);
}

void Resources::addGraphicsPipeline(std::string_view name, const shared<GraphicsPipeline>& graphics_pipeline)
{
    graphics_pipelines.insert_or_assign(name, graphics_pipeline);
}

void Resources::addComputePipeline(std::string_view name, const shared<ComputePipeline>& compute_pipeline)
{
    compute_pipelines.insert_or_assign(name, compute_pipeline);
}

void Resources::addImage(std::string_view name, const shared<Image2D>& image)
{
    images.insert_or_assign(name, image);
}

void Resources::addFont(std::string_view name, const shared<Font>& font)
{
    fonts.insert_or_assign(name, font);
}

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
