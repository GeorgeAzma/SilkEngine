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

Scene::Scene()
{
	Dispatcher::subscribe(this, &Scene::onWindowResize);

	registry.on_construct<RenderComponent>().connect<&Scene::onRenderComponentCreate>(this);
	registry.on_destroy<RenderComponent>().connect<&Scene::onRenderComponentDestroy>(this);

	indirect_buffer = std::make_shared<IndirectBuffer>(Graphics::MAX_BATCHES * sizeof(VkDrawIndexedIndirectCommand));
	
	shared<DescriptorSet> global_descriptor_set = makeShared<DescriptorSet>();
	global_descriptor_set->addBuffer(0, { *Graphics::global_uniform, 0, VK_WHOLE_SIZE }, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build();

	auto white_image = Resources::getImage("White");
	auto null_image = Resources::getImage("Null");
	shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>();
	descriptor_set->addImages(0, {
			*null_image, * white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image,
			*white_image, *white_image, *white_image, *white_image
		}, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();

	material_data = makeShared<MaterialData>(Resources::getMaterial("3D"), std::vector<shared<DescriptorSet>>{ global_descriptor_set, descriptor_set });
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
	{
		Graphics::global_uniform->setDataChecked(&main_camera->camera.projection_view, sizeof(glm::mat4), 0);
	}

	if (indirect_batches.size())
	{
		std::vector<VkDrawIndexedIndirectCommand> draw_commands;
		for (auto& batch : indirect_batches)
		{
			if (batch.needs_update && batch.instance_datas.size())
			{
				batch.render_object.mesh->vertex_array->getVertexBuffer(1)->setData(batch.instance_datas.data(), batch.instance_datas.size() * sizeof(InstanceData), 0);
				batch.needs_update = false;
			}
			VkDrawIndexedIndirectCommand draw_indexed_indirect_command{};
			draw_indexed_indirect_command.firstIndex = 0;
			draw_indexed_indirect_command.firstInstance = 0;
			draw_indexed_indirect_command.indexCount = batch.render_object.mesh->indices.size();
			draw_indexed_indirect_command.instanceCount = batch.instance_datas.size();
			draw_indexed_indirect_command.vertexOffset = 0;
			draw_commands.emplace_back(std::move(draw_indexed_indirect_command));
		}

		indirect_buffer->setData(draw_commands.data(), draw_commands.size() * sizeof(draw_commands[0]));

		Graphics::swap_chain->beginRenderPass();
		for (size_t i = 0; i < indirect_batches.size(); ++i)
		{
			if (!draw_commands[i].instanceCount)
				continue;

			indirect_batches[i].render_object.material_data->material->pipeline->bind();
			indirect_batches[i].render_object.mesh->vertex_array->bind();
			VkDescriptorSetLayoutBinding b{};
			b.binding = 0;
			b.descriptorCount = 1;
			b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			b.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			for (size_t j = 0; j < indirect_batches[i].render_object.material_data->descriptor_sets.size(); ++j)
				indirect_batches[i].render_object.material_data->descriptor_sets[j]->bind(j);
			
			vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, i * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
		}
		Graphics::swap_chain->endRenderPass();
		Graphics::swap_chain->endFrame();
	}
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
	InstanceData instance_data{};

	if (registry.any_of<TransformComponent>(entity))
		instance_data.transform = registry.get<TransformComponent>(entity).transform;

	if (registry.any_of<SpriteComponent>(entity))
		instance_data.texture_index = registry.get<SpriteComponent>(entity).texture_index;

	if (registry.any_of<ColorComponent>(entity))
		instance_data.color = registry.get<ColorComponent>(entity).color;

	render_component.render_object.instance_data = std::move(instance_data);
	render_component.render_object.material_data = material_data;
	addBatchRenderObject(render_component.render_object);
}

void Scene::onRenderComponentDestroy(entt::registry& registry, entt::entity entity) //TODO: Fix the error
{
	if (stopped)
		return;

	auto& render_component = registry.get<RenderComponent>(entity);
	removeBatchRenderObject(render_component.render_object);
}

void Scene::addBatchRenderObject(RenderObject render_object)
{
	for (size_t i = 0; i < indirect_batches.size(); ++i)
	{
		if (indirect_batches[i].render_object == render_object)
		{
			indirect_batches[i].instance_datas.emplace_back(render_object.instance_data);
			indirect_batches[i].needs_update = true;
			return;
		}
	}

	indirect_batches.emplace_back(render_object, std::vector<InstanceData>{render_object.instance_data});
}

void Scene::removeBatchRenderObject(const RenderObject& render_object)
{
	for (size_t i = 0; i < indirect_batches.size(); ++i)
	{
		if (indirect_batches[i].render_object == render_object)
		{
			//TODO: This is slow
			for (size_t j = 0; j < indirect_batches[i].instance_datas.size(); ++j)
			{
				if (indirect_batches[i].instance_datas[j] == render_object.instance_data)
				{
					std::swap(indirect_batches[i].instance_datas[j], indirect_batches[i].instance_datas.back());
					indirect_batches[i].instance_datas.pop_back();
					indirect_batches[i].needs_update = true;
					if (indirect_batches[i].instance_datas.empty())
					{
						std::swap(indirect_batches[i], indirect_batches.back());
						indirect_batches.pop_back();
					}
					return;
				}
			}
			return;
		}
	}
}