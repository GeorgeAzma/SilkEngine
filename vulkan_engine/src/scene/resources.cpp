#include "resources.h"
#include "meshes/circle_mesh.h"
#include "gfx/graphics.h"
#include "gfx/buffers/uniform_buffer.h"

void Resources::init()
{
	addMesh("Circle", std::make_shared<CircleMesh>());

    std::shared_ptr<DescriptorSetLayout> descriptor_set_layout = std::make_shared<DescriptorSetLayout>();
    descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    std::shared_ptr<Shader> shader(
        new Shader({
        "data/cache/shaders/3D.vert.spv",
        "data/cache/shaders/3D.frag.spv"
            })); //1.65ms

    std::shared_ptr<GraphicsPipeline> graphics_pipeline = std::make_shared<GraphicsPipeline>(); //1.35ms
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .addDescriptorSetLayout(*descriptor_set_layout)
        .setShader(shader)
        .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 } })
        .setSampleCount(Graphics::swap_chain->getSampleCount())
        .setRenderPass(Graphics::swap_chain->getRenderPass())
        .build();

	addMaterial("3D", std::make_shared<Material>(descriptor_set_layout, graphics_pipeline));
}

void Resources::cleanup()
{
	meshes.clear();
	materials.clear();
}

std::shared_ptr<Mesh> Resources::getMesh(const std::string& name)
{
	return meshes.at(name);
}

std::shared_ptr<Material> Resources::getMaterial(const std::string& name)
{
    return materials.at(name);
}


void Resources::addMesh(const std::string& name, std::shared_ptr<Mesh> mesh)
{
	mesh->name = name;
	meshes[name] = mesh;
}

void Resources::addMaterial(const std::string& name, std::shared_ptr<Material> material)
{
	materials[name] = material;
}
