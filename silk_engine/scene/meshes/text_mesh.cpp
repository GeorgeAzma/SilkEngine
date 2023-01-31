#include "text_mesh.h"
#include "gfx/ui/font.h"
#include "scene/resources.h"

TextMesh::TextMesh(std::string_view text, uint32_t size, shared<Font> font)
{
	auto layout = font->getCharacterLayout(text);

	size_t vertex_index = 0;
	size_t indices_index = 0;
	resizeVertices(layout.size() * 4);
	resizeIndices(layout.size() * 6);

	for (auto& c : layout)
	{
		getIndex(indices_index++) = vertex_index + 2;
		getIndex(indices_index++) = vertex_index + 1;
		getIndex(indices_index++) = vertex_index + 0;
		getIndex(indices_index++) = vertex_index + 0;
		getIndex(indices_index++) = vertex_index + 3;
		getIndex(indices_index++) = vertex_index + 2;

		getVertex(vertex_index).position = vec2(c.position.x, c.position.y);
		getVertex(vertex_index).texture_coordinate = vec2(c.texture_coordinate.x, c.texture_coordinate.w);
		++vertex_index;
		getVertex(vertex_index).position = vec2(c.position.x, c.position.w);
		getVertex(vertex_index).texture_coordinate = vec2(c.texture_coordinate.x, c.texture_coordinate.y);
		++vertex_index;
		getVertex(vertex_index).position = vec2(c.position.z, c.position.w);
		getVertex(vertex_index).texture_coordinate = vec2(c.texture_coordinate.z, c.texture_coordinate.y);
		++vertex_index;
		getVertex(vertex_index).position = vec2(c.position.z, c.position.y);
		getVertex(vertex_index).texture_coordinate = vec2(c.texture_coordinate.z, c.texture_coordinate.w);
		++vertex_index;
	}
}