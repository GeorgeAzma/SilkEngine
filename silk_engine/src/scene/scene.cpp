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

	CameraComponent* main_camera = nullptr;
	registry.view<CameraComponent>().each(
		[&](auto entity, auto& camera_component)
		{
			main_camera = &camera_component;
		});
	if (main_camera)
		Graphics::global_uniform->setDataChecked(&main_camera->camera.projection_view, sizeof(glm::mat4), 0);

	//Drawing
	Graphics::swap_chain->beginRenderPass();
	
	//Draw instances
	for (auto& instance_batch : instance_batches)
	{
		if (instance_batch.needs_update && instance_batch.instance_data.size())
		{
			instance_batch.instance.mesh->vertex_array->getVertexBuffer(1)->setData(instance_batch.instance_data.data(), instance_batch.instance_data.size() * sizeof(InstanceData));
			instance_batch.needs_update = false;
		}

		instance_batch.instance.material_data->material->pipeline->bind();
		instance_batch.instance.mesh->vertex_array->bind();
		for (size_t j = 0; j < instance_batch.instance.material_data->descriptor_sets.size(); ++j)
			instance_batch.instance.material_data->descriptor_sets[j]->bind(j);
		
		//NOTE: vkCmdDrawIndexedIndirect is bit faster but it limits us by having a fixed instanced batches size and adds clutter
		vkCmdDrawIndexed(Graphics::active.command_buffer, instance_batch.instance.mesh->indices.size(), instance_batch.instance_data.size(), 0, 0, 0);
	}
	Graphics::stats.instances = instance_batches.size();
	Graphics::stats.batches = Renderer::batcher.batches.size();

	Renderer::updateBatch();
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
		[](auto entity, auto& camera_component)
		{
			camera_component.camera.onViewportResize();
		});
}

void Scene::onRenderComponentCreate(entt::registry& registry, entt::entity entity)
{
	RenderComponent& render_component = registry.get<RenderComponent>(entity);
	
	InstanceData instance_data{};

	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;

	if (auto sprite = registry.try_get<SpriteComponent>(entity))
		instance_data.texture_index = *sprite;

	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;

	if (Renderer::batcher.active)
	{
		render_component.instance.material_data = material_data_batch3D;
		render_component.instance.batched = true;
		render_component.instance.instance_data = &instance_data;

		addBatchedInstance(render_component.instance);
	}
	else
	{
		render_component.instance.material_data = material_data_3D;
		render_component.instance.batched = false;

		for (auto& instance_batch : instance_batches)
		{
			if (instance_batch == render_component.instance)
			{	
				instance_batch.needs_update = true;
				
				instance_batch.instance_data.emplace_back(std::move(instance_data));
				render_component.instance.instance_data = &instance_batch.instance_data.back();

				return;
			}
		}

		std::vector<InstanceData> new_instance_data;
		new_instance_data.reserve(Graphics::MAX_INSTANCES);
		new_instance_data.emplace_back(std::move(instance_data));
		instance_batches.emplace_back(render_component.instance, std::move(new_instance_data));
		render_component.instance.instance_data = &instance_batches.back().instance_data.back();
	}
}

void Scene::onRenderComponentDestroy(entt::registry& registry, entt::entity entity)
{
	if (stopped)
		return;

	RenderComponent& render_component = registry.get<RenderComponent>(entity);
	
	for (auto& instance_batch : instance_batches)
	{
		if (instance_batch.instance == render_component.instance)
		{
			instance_batch.needs_update = true;

			registry.view<RenderComponent>().each(
				[&](entt::entity entity, RenderComponent& rc) 
				{
					if (rc.instance.instance_data == &instance_batch.instance_data.back())
						rc.instance.instance_data = render_component.instance.instance_data;
				});
			std::swap(*render_component.instance.instance_data, instance_batch.instance_data.back());
			instance_batch.instance_data.pop_back();

			return;
		}
	}
}

template<>
void Scene::updateComponent<RenderComponent>(entt::entity entity)
{
	RenderComponent& render_component = registry.get<RenderComponent>(entity);
	
	InstanceData instance_data{};
	
	if (auto transform = registry.try_get<TransformComponent>(entity))
		instance_data.transform = *transform;
	
	if (auto sprite = registry.try_get<SpriteComponent>(entity))
		instance_data.texture_index = *sprite;
	
	if (auto color = registry.try_get<ColorComponent>(entity))
		instance_data.color = *color;
	
	*render_component.instance.instance_data = std::move(instance_data);

	if (render_component.instance.batched)
	{
		for (auto& batch : Renderer::batcher.batches)
		{
			if (batch == render_component.instance)
			{
				batch.needs_update = true;

				break;
			}
		}
	}
	else
	{
		for (auto& instance_batch : instance_batches)
		{
			if (instance_batch.instance == render_component.instance)
			{
				instance_batch.needs_update = true;

				break;
			}
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

	Renderer::batcher.batches.back().addInstance(instance);
}