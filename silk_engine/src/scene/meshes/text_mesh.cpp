#include "text_mesh.h"
#include "gfx/ui/font.h"
#include "scene/resources.h"

TextMesh::TextMesh(const std::string& text, uint32_t size, shared<Font> font)
{
	font = font.get() ? font : Resources::getFont("Arial");
	auto layout = font->getCharacterLayout(text);

	size_t vertex_index = 0;
	size_t indices_index = 0;
	indices.resize(layout.size() * 6);
	vertices.resize(layout.size() * 4);
	for (auto& c : layout)
	{
		indices[indices_index++] = vertex_index + 2;
		indices[indices_index++] = vertex_index + 1;
		indices[indices_index++] = vertex_index + 0;
		indices[indices_index++] = vertex_index + 0;
		indices[indices_index++] = vertex_index + 3;
		indices[indices_index++] = vertex_index + 2;

		vertices[vertex_index].position = glm::vec2(c.position.x, c.position.y);
		vertices[vertex_index].texture_coordinates = glm::vec2(c.texture_coordinate.x, c.texture_coordinate.w);
		++vertex_index;
		vertices[vertex_index].position = glm::vec2(c.position.x, c.position.w);
		vertices[vertex_index].texture_coordinates = glm::vec2(c.texture_coordinate.x, c.texture_coordinate.y);
		++vertex_index;
		vertices[vertex_index].position = glm::vec2(c.position.z, c.position.w);
		vertices[vertex_index].texture_coordinates = glm::vec2(c.texture_coordinate.z, c.texture_coordinate.y);
		++vertex_index;
		vertices[vertex_index].position = glm::vec2(c.position.z, c.position.y);
		vertices[vertex_index].texture_coordinates = glm::vec2(c.texture_coordinate.z, c.texture_coordinate.w);
		++vertex_index;
	}
}