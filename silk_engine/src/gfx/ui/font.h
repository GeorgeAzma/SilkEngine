#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H  
#include "gfx/images/image.h"

class Font
{
	struct Character
	{
		glm::ivec2 size;
		glm::ivec2 bearing;
		glm::uvec2 advance;
		glm::vec2 texture_coordinate;
	};
public:
	static void init();
	static void cleanup();

public:
	Font(const std::string& file, size_t size = 64);
	~Font();

	const std::string& getPath() const { return path; }

public:
	static constexpr size_t MAX_CHARACTER_COUNT = 256;

private:
	static inline FT_Library free_type_library;

private:
	FT_Face face;
	std::string path;
	std::vector<Character> characters;
	shared<Image> texture_atlas;
};