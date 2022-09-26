#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H  
#include "gfx/images/image.h"

class Font
{
	struct CharacterInfo
	{
		ivec2 size;
		ivec2 bearing;
		uvec2 advance;
		vec4 texture_coordinate;
	};

	struct Character
	{
		vec4 position;
		vec4 texture_coordinate;
	};

public:
	static void init();
	static void destroy();

public:
	Font(std::string_view file, uint32_t size = 64);
	~Font();

	std::string_view getPath() const { return path; }
	shared<Image> getAtlas() const { return texture_atlas; }

	std::vector<Character> getCharacterLayout(std::string_view str);

public:
	static constexpr size_t MAX_CHARACTER_COUNT = 256;

private:
	static inline FT_Library free_type_library;

private:
	FT_Face face;
	std::string path;
	std::vector<CharacterInfo> characters;
	size_t size;

	shared<Image> texture_atlas;
};