#pragma once

#include "instance.h"
#include "camera/camera.h"

class Entity;
class WindowResizeEvent;

class Scene
{
	friend class Entity;

public:
	Scene();
	~Scene();

	void onPlay();
	void onUpdate();
	void onStop();

	shared<Entity> createEntity();
	void removeEntity(const entt::entity& entity);

	void onTransformComponentUpdate(entt::registry& registry, entt::entity entity);
	void onColorComponentUpdate(entt::registry& registry, entt::entity entity);
	void onMaterialComponentUpdate(entt::registry& registry, entt::entity entity);
	void onLightComponentUpdate(entt::registry& registry, entt::entity entity);
	void onImageComponentUpdate(entt::registry& registry, entt::entity entity);
	void onTextComponentUpdate(entt::registry& registry, entt::entity entity);

	Camera* getMainCamera();

private:
	void onWindowResize(const WindowResizeEvent& e);
	
	void onMeshComponentCreate(entt::registry& registry, entt::entity entity);
	void onTextComponentCreate(entt::registry& registry, entt::entity entity);
	void onModelComponentCreate(entt::registry& registry, entt::entity entity);	
	void onLightComponentCreate(entt::registry& registry, entt::entity entity);
	
	void onMeshComponentDestroy(entt::registry& registry, entt::entity entity);
	void onModelComponentDestroy(entt::registry& registry, entt::entity entity);
	void onLightComponentDestroy(entt::registry& registry, entt::entity entity);

private:
	entt::registry registry;
};










//GPU CULL: Slower than brute force rendering, makes sense but WTF, I just spent a day to make this
/*
shared<StorageBuffer> cull_data_buffer;
shared<StorageBuffer> rendered_buffer;
shared<DescriptorSet> cull_descriptor_set;

cull_data_buffer = makeShared<StorageBuffer>(Graphics::MAX_INSTANCES * sizeof(glm::mat4));
rendered_buffer = makeShared<StorageBuffer>(Graphics::MAX_INSTANCES * sizeof(VkBool32));
cull_descriptor_set = makeShared<DescriptorSet>(*Resources::getComputeMaterial("Cull")->descriptor_set_layout);
cull_descriptor_set->addBuffer(0, { *indirect_buffer, 0, VK_WHOLE_SIZE })
	.addBuffer(1, { *cull_data_buffer, 0, VK_WHOLE_SIZE })
	.addBuffer(2, { *rendered_buffer, 0, VK_WHOLE_SIZE })
	.build();

VkBufferMemoryBarrier buffer_barrier{};
buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
buffer_barrier.offset = 0;
buffer_barrier.buffer = *indirect_buffer;
buffer_barrier.size = indirect_batches.size() * sizeof(VkDrawIndexedIndirectCommand);
buffer_barrier.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
buffer_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
buffer_barrier.srcQueueFamilyIndex = *Graphics::physical_device->getQueueFamilyIndices().graphics;
buffer_barrier.dstQueueFamilyIndex = *Graphics::physical_device->getQueueFamilyIndices().graphics;
vkCmdPipelineBarrier(
	Graphics::active.command_buffer,
	VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	0,
	0, nullptr,
	1, &buffer_barrier,
	0, nullptr);

Resources::getComputeMaterial("Cull")->pipeline->bind();
cull_descriptor_set->bind();
auto layout = Resources::getComputeMaterial("Cull")->pipeline->getLayout();
size_t index = 0;
for (size_t i = 0; i < indirect_batches.size(); ++i)
{
	auto& batch = indirect_batches[i];
	CullData cull_data{};
	cull_data.min = batch.instance.mesh->aabb.min;
	cull_data.index = index++;
	cull_data.max = batch.instance.mesh->aabb.max;
	cull_data.count = batch.instance_datas.size();
	memcpy(cull_data.planes.data(), main_camera->camera.frustum.getPlanes().data(), cull_data.planes.size() * sizeof(glm::vec4));

	std::vector<glm::mat4> transforms(batch.instance_datas.size());
	for (size_t j = 0; j < batch.instance_datas.size(); ++j)
	{
		transforms[j] = batch.instance_datas[j].transform;
	}
	cull_data_buffer->setData(transforms.data(), transforms.size() * sizeof(glm::mat4));

	vkCmdPushConstants(Graphics::active.command_buffer, layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullData), &cull_data);
	vkCmdDispatch(Graphics::active.command_buffer, indirect_batches[i].instance_datas.size() / 64 + (indirect_batches[i].instance_datas.size() % 64 > 0), 1, 1);
}

VkBufferMemoryBarrier barrier{};
barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
barrier.buffer = *indirect_buffer;
barrier.offset = 0;
barrier.size = indirect_batches.size() * sizeof(VkDrawIndexedIndirectCommand);
barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
barrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
barrier.dstQueueFamilyIndex = *Graphics::physical_device->getQueueFamilyIndices().graphics;
barrier.srcQueueFamilyIndex = *Graphics::physical_device->getQueueFamilyIndices().graphics;
vkCmdPipelineBarrier(Graphics::active.command_buffer,
	VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
	0, 0, nullptr, 1, &barrier, 0, nullptr);

for (size_t i = 0; i < indirect_batches.size(); ++i)
{
	std::vector<VkBool32> rendered;
	rendered_buffer->getData(rendered, indirect_batches[i].instance_datas.size() * sizeof(VkBool32));

	std::vector<InstanceData> rendered_instances;
	rendered_instances.reserve(indirect_batches[i].instance_datas.size());
	for (size_t j = 0; j < indirect_batches[i].instance_datas.size(); ++j)
	{
		if (rendered[j])
		{
			rendered_instances.emplace_back(indirect_batches[i].instance_datas[j]);
		}
	}
	if (rendered_instances.size())
		indirect_batches[i].instance.mesh->vertex_array->getVertexBuffer(1)->setData(rendered_instances.data(), rendered_instances.size() * sizeof(InstanceData));
	SK_INFO(rendered_instances.size());
}
*/
