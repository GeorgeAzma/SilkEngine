#include "resources.h"
#include "meshes/circle_mesh.h"
#include "meshes/rectangle_mesh.h"
#include "gfx/graphics.h"
#include "gfx/buffers/uniform_buffer.h"

void Resources::init()
{
	addMesh("Circle", makeShared<CircleMesh>());
	addMesh("Rectangle", makeShared<RectangleMesh>());

    shared<DescriptorSetLayout> descriptor_set_layout = makeShared<DescriptorSetLayout>();
    descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    shared<Shader> shader(new Shader({"data/cache/shaders/3D.vert.spv", "data/cache/shaders/3D.frag.spv"})); //1.65ms
    shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>(); //1.35ms
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .addDescriptorSetLayout(*descriptor_set_layout)
        .setShader(shader)
        .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 } })
        .setSampleCount(Graphics::swap_chain->getSampleCount())
        .setRenderPass(Graphics::swap_chain->getRenderPass())
        .build();
	addMaterial("3D", makeShared<Material>(descriptor_set_layout, graphics_pipeline));
    
    descriptor_set_layout = makeShared<DescriptorSetLayout>();
    descriptor_set_layout->addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
        .build();
    shader = shared<Shader>(new Shader({ "data/cache/shaders/cull.comp.spv" })); //1.65ms
    shared<ComputePipeline> compute_pipeline = makeShared<ComputePipeline>(); //1.35ms
    compute_pipeline->addDescriptorSetLayout(*descriptor_set_layout)
        .setShader(shader)
        .build();
    addComputeMaterial("Cull", makeShared<ComputeMaterial>(descriptor_set_layout, compute_pipeline));
}

void Resources::cleanup()
{
	meshes.clear();
	materials.clear();
	compute_materials.clear();
}

shared<Mesh> Resources::getMesh(const std::string& name)
{
	return meshes.at(name);
}

shared<Material> Resources::getMaterial(const std::string& name)
{
    return materials.at(name);
}

shared<ComputeMaterial> Resources::getComputeMaterial(const std::string& name)
{
    return compute_materials.at(name);
}

void Resources::addMesh(const std::string& name, shared<Mesh> mesh)
{
	mesh->name = name;
	meshes[name] = mesh;
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
