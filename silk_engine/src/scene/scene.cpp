#include "scene.h"
#include "entity.h"
#include "components.h"
#include "gfx/graphics.h"
#include "scene/resources.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "gfx/buffers/uniform_buffer.h"
#include "utils/general_utils.h"
#include "gfx/window/swap_chain.h"
#include "gfx/renderer.h"

//shared<Font> font;

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);
	
	registry.on_construct<RenderComponent>().connect<&Scene::onRenderComponentCreate>(this);
	registry.on_destroy<RenderComponent>().connect<&Scene::onRenderComponentDestroy>(this);

	indirect_buffer = std::make_shared<IndirectBuffer>(Graphics::MAX_INSTANCE_BATCHES * sizeof(VkDrawIndexedIndirectCommand));
	
	shared<DescriptorSet> global_descriptor_set = makeShared<DescriptorSet>();
	global_descriptor_set->addBuffer(0, { *Graphics::global_uniform, 0, VK_WHOLE_SIZE }, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build();

	//font = makeShared<Font>("arial.ttf");
	auto white_image = Resources::getImage("White");
	shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>();
	descriptor_set->addImages(0, {
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image
		}, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();

	material_data_3D = makeShared<MaterialData>(Resources::getMaterial("3D"), std::vector<shared<DescriptorSet>>{ global_descriptor_set, descriptor_set });
	material_data_batch3D = makeShared<MaterialData>(Resources::getMaterial("Batch3D"), std::vector<shared<DescriptorSet>>{ global_descriptor_set, descriptor_set });
}

Scene::~Scene()
{
	Dispatcher::unsubscribe(this, &Scene::onWindowResize);
	registry.on_construct<RenderComponent>().disconnect<&Scene::onRenderComponentCreate>(this);
	registry.on_destroy<RenderComponent>().disconnect<&Scene::onRenderComponentDestroy>(this);
}

void Scene::onPlay()
{
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			if (!script_component.instance)
			{
				script_component.instance = script_component.instantiate_script();
				script_component.instance->entity = makeShared<Entity>(entity, this);

				script_component.instance->onCreate();
			}
		});
}

void Scene::onUpdate()
{
	Graphics::swap_chain->beginFrame();

	//Update components
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onUpdate();
		});

	//TODO: determine how to choose main camera
	CameraComponent* main_camera = nullptr;
	registry.view<CameraComponent>().each(
		[&](auto entity, auto& camera_component)
		{
			main_camera = &camera_component;
		});
	//TODO: not too scalable
	if (main_camera)
		Graphics::global_uniform->setDataChecked(&main_camera->camera.projection_view, sizeof(glm::mat4), 0);
	
	std::vector<VkDrawIndexedIndirectCommand> draw_commands;
	if (instances.size())
	{
		//Create draw commands and set rendered vertex buffer's instance data
		for (auto& batch : instances)
		{
			if (batch.needs_update && batch.rendered_instances.size())
			{
				std::vector<InstanceData> instance_datas(batch.rendered_instances.size());
				for (size_t i = 0; i < instance_datas.size(); ++i)
					instance_datas[i] = batch.rendered_instances[i]->instance_data;
				batch.instance.mesh->vertex_array->getVertexBuffer(1)->setData(instance_datas.data(), instance_datas.size() * sizeof(InstanceData));
				batch.needs_update = false;
			}
			VkDrawIndexedIndirectCommand draw_indexed_indirect_command{};
			draw_indexed_indirect_command.firstIndex = 0;
			draw_indexed_indirect_command.firstInstance = 0;
			draw_indexed_indirect_command.indexCount = batch.instance.mesh->indices.size();
			draw_indexed_indirect_command.instanceCount = batch.rendered_instances.size();
			draw_indexed_indirect_command.vertexOffset = 0;
			draw_commands.emplace_back(std::move(draw_indexed_indirect_command));
		}
		indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(draw_commands[0]));
	}
	Graphics::stats.instances = instances.size();
	Graphics::stats.batches = Renderer::batcher.batches.size();

	//Drawing
	Graphics::swap_chain->beginRenderPass();
	
	//Draw instances
	for (size_t i = 0; i < instances.size(); ++i)
	{
		if (!draw_commands[i].instanceCount)
			continue;

		instances[i].instance.material_data->material->pipeline->bind();
		instances[i].instance.mesh->vertex_array->bind();
		for (size_t j = 0; j < instances[i].instance.material_data->descriptor_sets.size(); ++j)
			instances[i].instance.material_data->descriptor_sets[j]->bind(j);
		
		vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, i * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
	}

	Renderer::drawLastBatch();
	
	//End draw
	Graphics::swap_chain->endRenderPass();

	Graphics::swap_chain->endFrame();

}

void Scene::onStop()
{
	stopped = true;
	registry.view<ScriptComponent>().each(
		[&](auto entity, auto& script_component)
		{
			script_component.instance->onDestroy();
		});
}

shared<Entity> Scene::createEntity()
{
	return makeShared<Entity>(registry.create(), this);
}

void Scene::removeEntity(const entt::entity& entity)
{
	registry.destroy(entity);
}

void Scene::onWindowResize(const WindowResizeEvent& e)
{
	registry.view<CameraComponent>().each(
		[&](auto entity, auto& camera_component)
		{
			camera_component.camera.onViewportResize();
		});
}

void Scene::onRenderComponentCreate(entt::registry& registry, entt::entity entity)
{
	auto& render_component = registry.get<RenderComponent>(entity);

	if (registry.any_of<TransformComponent>(entity))
		render_component.instance.instance_data.transform = registry.get<TransformComponent>(entity).transform;

	if (registry.any_of<SpriteComponent>(entity))
		render_component.instance.instance_data.texture_index = registry.get<SpriteComponent>(entity).texture_index;

	if (registry.any_of<ColorComponent>(entity))
		render_component.instance.instance_data.color = registry.get<ColorComponent>(entity).color;

	if (Renderer::batcher.active)
	{
		render_component.instance.material_data = material_data_batch3D;
		addBatchedInstance(render_component.instance);
	}
	else
	{
		render_component.instance.material_data = material_data_3D;
		addInstance(render_component.instance);
	}
}

void Scene::onRenderComponentDestroy(entt::registry& registry, entt::entity entity)
{
	if (stopped)
		return;

	auto& render_component = registry.get<RenderComponent>(entity);
	removeInstance(render_component.instance);
}

template<>
void Scene::updateComponent<RenderComponent>(entt::entity entity)
{
	SK_ASSERT(registry.any_of<RenderComponent>(entity),
		"Entity has no transform component, don't call updateComponent on non existant component");
	
	auto& render_component = registry.get<RenderComponent>(entity);
	
	if(registry.any_of<TransformComponent>(entity))
		render_component.instance.instance_data.transform = registry.get<TransformComponent>(entity);
	
	if (registry.any_of<SpriteComponent>(entity))
		render_component.instance.instance_data.texture_index = registry.get<SpriteComponent>(entity);
	
	if (registry.any_of<ColorComponent>(entity))
		render_component.instance.instance_data.color = registry.get<ColorComponent>(entity);

	for (size_t i = 0; i < instances.size(); ++i)
	{
		if (instances[i] == render_component.instance)
		{
			instances[i].needs_update = true;
			break;
		}
	}
}

void Scene::addBatchedInstance(const RenderedInstance& instance)
{
	const Mesh& mesh = *instance.mesh;

	SK_ASSERT(mesh.indices.size() < Graphics::MAX_BATCH_INDICES, 
		"Batched instance has too much indices. max indices allowed is: {0}, but mesh has {1}", 
		Graphics::MAX_BATCH_INDICES, mesh.indices.size());

	SK_ASSERT(mesh.vertices.size() < Graphics::MAX_BATCH_VERTICES, 
		"Batched instance has too much vertices. max verticees allowed is: {0}, but mesh has {1}", 
		Graphics::MAX_BATCH_VERTICES, mesh.vertices.size());

	bool should_add_batch = false;

	if (Renderer::batcher.batches.empty())
	{
		should_add_batch = true;
	}
	else if (instance.material_data != Renderer::batcher.batches.back().material_data)
	{
		for (size_t i = 0; i < Renderer::batcher.batches.size(); ++i)
		{
			if (Renderer::batcher.batches[i].material_data == instance.material_data)
			{
				std::swap(Renderer::batcher.batches[i], Renderer::batcher.batches.back());
				break;
			}
		}
	}

	//If batch had not enough vertices/indices to store the mesh create a new batch
	if (!Renderer::batcher.batches.empty() && 
		(Renderer::batcher.batches.back().vertices_index + mesh.vertices.size() >= Renderer::batcher.batches.back().vertices.size() ||
		Renderer::batcher.batches.back().indices_index + mesh.indices.size() >= Renderer::batcher.batches.back().indices.size()))
	{
		should_add_batch = true;
	}

	if (should_add_batch)
	{
		Renderer::addBatch();
		Renderer::batcher.batches.back().material_data = instance.material_data;
	}

	Renderer::batcher.batches.back().addInstance(instance, Renderer::batcher.index_offset);
	Renderer::batcher.index_offset += instance.mesh->vertices.size();
}

void Scene::addInstance(RenderedInstance& instance)
{
	for (size_t i = 0; i < instances.size(); ++i)
	{
		if (instances[i].instance == instance)
		{
			instances[i].rendered_instances.emplace_back(&instance);
			instances[i].needs_update = true;
			return;
		}
	}

	instances.emplace_back(instance, std::vector<RenderedInstance*>{&instance});
}

void Scene::removeInstance(const RenderedInstance& instance)
{
	for (size_t i = 0; i < instances.size(); ++i)
	{
		if (instances[i].instance == instance)
		{
			//TODO: This is bit slow
			for (size_t j = 0; j < instances[i].rendered_instances.size(); ++j)
			{
				if (instances[i].rendered_instances[j] == &instance)
				{
					std::swap(instances[i].rendered_instances[j], instances[i].rendered_instances.back());
					instances[i].rendered_instances.pop_back();
					instances[i].needs_update = true;
					if (instances[i].rendered_instances.empty())
					{
						std::swap(instances[i], instances.back());
						instances.pop_back();
					}
					return;
				}
			}
			return;
		}
	}
}