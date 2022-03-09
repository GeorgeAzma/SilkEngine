#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H  
#include "gfx/images/image2D.h"

class Font
{
	struct CharacterInfo
	{
		glm::ivec2 size;
		glm::ivec2 bearing;
		glm::uvec2 advance;
		glm::vec4 texture_coordinate;
	};

	struct Character
	{
		glm::vec4 position;
		glm::vec4 texture_coordinate;
	};

public:
	static void init();
	static void cleanup();

public:
	Font(const std::string& file, uint32_t size = 64);
	~Font();

	const std::string& getPath() const { return path; }
	shared<Image2D> getAtlas() const { return texture_atlas; }

	std::vector<Character> getCharacterLayout(const std::string& str);

public:
	static constexpr size_t MAX_CHARACTER_COUNT = 256;

private:
	static inline FT_Library free_type_library;

private:
	FT_Face face;
	std::string path;
	std::vector<CharacterInfo> characters;
	size_t size;

	shared<Image2D> texture_atlas;
};