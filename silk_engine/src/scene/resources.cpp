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

void Resources::init()
{
    //CREATE NEEDED DIRECTORIES AND INIT STUFF
    {
        if (!std::filesystem::exists("data/cache/shaders"))
            std::filesystem::create_directories("data/cache/shaders");
    }

    //IMAGES
    {       
        addImage("White", [] 
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
                return makeShared<Image2D>(image_props); 
            });

        white_image = getImage("White");

        addImage("Black", [] 
            { 
                Image2DProps image_props{};
                image_props.width = 1;
                image_props.height = 1;
                image_props.sampler_props.min_filter = vk::Filter::eNearest;
                image_props.sampler_props.mag_filter = vk::Filter::eNearest;
                image_props.sampler_props.linear_mipmap = false;
                image_props.sampler_props.anisotropy = false;
                image_props.mipmap = false;
                constexpr glm::u8vec4 black(0);
                image_props.data = &black;
                return makeShared<Image2D>(image_props); 
            });

        addImage("Null", [] 
            { 
                Image2DProps image_props{};
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
                return makeShared<Image2D>(image_props); 
            });
    }

    //MESHES
    {
        addMesh("Triangle", [] { return makeShared<TriangleMesh>(); });
        addMesh("Circle", [] { return makeShared<CircleMesh>(); });
        addMesh("Rectangle", [] { return makeShared<RectangleMesh>(); });
        addMesh("Quad", [] { return makeShared<QuadMesh>(); });
        addMesh("Circle3D", [] { return shared<Mesh3D>(*(Mesh2D*)(getMesh("Circle").get())); });
        addMesh("Rectangle3D", [] { return shared<Mesh3D>(*(Mesh2D*)(getMesh("Rectangle").get())); });
        addMesh("Cube", [] { return makeShared<CubeMesh>(); });
        addMesh("Sphere", [] { return makeShared<SphereMesh>(); });
        addMesh("Tetrahedron", [] { return makeShared<TetrahedronMesh>(); });
    }

    //MODELS
    {
    }

    //FONTS
    {
        addFont("Arial", [] { return makeShared<Font>("arial.ttf"); });
    }

    //SHADERS
    {
        addShader("3D", [] { return makeShared<Shader>("3D"); });
        addShader("2D", [] { return makeShared<Shader>("2D"); });
        addShader("Font", [] { return makeShared<Shader>("font"); });
        addShader("BGRA To RGBA", [] { return makeShared<Shader>("bgra_to_rgba"); });
    }

    //PIPELINES
    {       
        addGraphicsPipeline("Lit 3D", [] 
            { 
                shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
                graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
                    .enable(EnableTag::DEPTH_TEST)
                    .enable(EnableTag::DEPTH_WRITE)
                    .setShader(getShader("3D"))
                    .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::VEC4 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
                    .setSampleCount(Graphics::swap_chain->getSampleCount())
                    .setRenderPass(*Graphics::swap_chain->getRenderPass())
                    .build();
                return graphics_pipeline; 
            });

        addGraphicsPipeline("3D", [] 
            {
                vk::Bool32 lit = false;
                shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
                graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
                    .enable(EnableTag::DEPTH_TEST)
                    .enable(EnableTag::DEPTH_WRITE)
                    .setShader(getShader("3D"), { { "lit", &lit, sizeof(vk::Bool32) } })
                    .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::VEC4 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
                    .setSampleCount(Graphics::swap_chain->getSampleCount())
                    .setRenderPass(*Graphics::swap_chain->getRenderPass())
                    .build();
                return graphics_pipeline; 
            });

        addGraphicsPipeline("2D", []
            { 
                shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
                graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
                    .enable(EnableTag::DEPTH_TEST)
                    .enable(EnableTag::DEPTH_WRITE)
                    .setDepthCompareOp(vk::CompareOp::eAlways)
                    .setShader(getShader("2D"))
                    .setVertexLayout({ { Type::VEC2 }, { Type::VEC2 }, { Type::VEC4 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
                    .setSampleCount(Graphics::swap_chain->getSampleCount())
                    .setRenderPass(*Graphics::swap_chain->getRenderPass())
                    .build();
                return graphics_pipeline; 
            });

        
        addGraphicsPipeline("Font", [] 
            {
                shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
                graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
                    .enable(EnableTag::DEPTH_TEST)
                    .enable(EnableTag::DEPTH_WRITE)
                    .setDepthCompareOp(vk::CompareOp::eAlways)
                    .setShader(getShader("Font"))
                    .setVertexLayout({ { Type::VEC2 }, { Type::VEC2 }, { Type::VEC4 }, { Type::MAT4, 1 }, { Type::UINT, 1 }, { Type::VEC4, 1 } })
                    .setSampleCount(Graphics::swap_chain->getSampleCount())
                    .setRenderPass(*Graphics::swap_chain->getRenderPass())
                    .build();
                return graphics_pipeline;
            });

        addComputePipeline("BGRA To RGBA", [] { return makeShared<ComputePipeline>(getShader("BGRA To RGBA")); });
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
    return fetch(meshes, name);
}

shared<Model> Resources::getModel(std::string_view name)
{
    return fetch(models, name);
}

shared<Shader> Resources::getShader(std::string_view name)
{
    return fetch(shaders, name);
}

shared<GraphicsPipeline> Resources::getGraphicsPipeline(std::string_view name)
{
    return fetch(graphics_pipelines, name);
}

shared<ComputePipeline> Resources::getComputePipeline(std::string_view name)
{
    return fetch(compute_pipelines, name);
}

shared<Image2D> Resources::getImage(std::string_view name)
{
    return fetch(images, name);
}

shared<Font> Resources::getFont(std::string_view name)
{
    return fetch(fonts, name);
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

void Resources::addMesh(std::string_view name, const std::function<shared<Mesh>()>& mesh)
{
    add(meshes, name, mesh);
}

void Resources::addModel(std::string_view name, const std::function<shared<Model>()>& model)
{
    add(models, name, model);
}

void Resources::addShader(std::string_view name, const std::function<shared<Shader>()>& shader)
{
    add(shaders, name, shader);
}

void Resources::addGraphicsPipeline(std::string_view name, const std::function<shared<GraphicsPipeline>()>& graphics_pipeline)
{
    add(graphics_pipelines, name, graphics_pipeline);
}

void Resources::addComputePipeline(std::string_view name, const std::function<shared<ComputePipeline>()>& compute_pipeline)
{
    add(compute_pipelines, name, compute_pipeline);
}

void Resources::addImage(std::string_view name, const std::function<shared<Image2D>()>& image)
{
    add(images, name, image);
}

void Resources::addFont(std::string_view name, const std::function<shared<Font>()>& font)
{
    add(fonts, name, font);
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

    for (const vk::DescriptorSetLayoutBinding& b : bindings)
    {
        size_t binding_hash = b.binding | (VkDescriptorType(b.descriptorType) << 8) | (uint32_t(b.descriptorCount) << 16) | (VkShaderStageFlags(b.stageFlags) << 24);
        result ^= std::hash<size_t>()(binding_hash);
    }

    return result;
}
