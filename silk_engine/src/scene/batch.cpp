#include "batch.h"

void Batch::addInstance(const RenderedInstance& instance, uint32_t index_offset)
{
	//TODO:
	//for (size_t i = 0; i < instance.mesh->vertices.size(); ++i)
	//{
	//	BatchVertex batch_vertex{};
	//	batch_vertex.position = instance.instance_data.transform * glm::vec4(instance.mesh->vertices[i].position, 1);
	//	batch_vertex.texture_coordinates = instance.mesh->vertices[i].texture_coordinates;
	//	batch_vertex.normal = instance.mesh->vertices[i].normal;
	//	batch_vertex.texture_index = instance.instance_data.texture_index;
	//	batch_vertex.color = instance.instance_data.color;
	//	addVertex(std::move(batch_vertex));
	//}
	//
	//for (size_t i = 0; i < instance.mesh->indices.size(); ++i)
	//{
	//	addIndex(index_offset + instance.mesh->indices[i]);
	//}
	//
	//needs_update = true;
}