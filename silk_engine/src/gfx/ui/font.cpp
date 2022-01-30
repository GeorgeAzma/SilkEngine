#include "font.h"
#include "gfx/images/image.h"

void Font::init()
{
	FT_Error result = FT_Init_FreeType(&free_type_library);
	SK_ASSERT(!result, "FreeType: Couldn't initialize free type");
}

void Font::cleanup()
{
	FT_Done_FreeType(free_type_library);
}

Font::Font(const std::string& file, size_t size)
	: characters(MAX_CHARACTER_COUNT)
{
	path = std::string("data/fonts/") + file;
	FT_Error result = FT_New_Face(free_type_library, path.c_str(), 0, &face);
	SK_ASSERT(!result, "FreeType: Couldn't create new face");

	FT_Set_Pixel_Sizes(face, 0, size);

	//Get texture atlas dimensions first
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int row_width = 0;
	unsigned int row_height = 0;
	
	//This is just random assumption to make aspect ratio as tight as possible
	const unsigned int max_width = size * sqrt(MAX_CHARACTER_COUNT); 
	SK_ASSERT(max_width > 0, "FreeType: Invalid texture input, it is too small");

	for (size_t i = 0; i < MAX_CHARACTER_COUNT; ++i)
	{
		FT_Error result = FT_Load_Char(face, i, FT_LOAD_RENDER);
		SK_ASSERT(!result, "FreeType: Couldn't load character: {0}, from file {1}", i, path);

		if (row_width + face->glyph->bitmap.width + 1 >= max_width)
		{
			width = std::max(width, row_width);
			height += row_height;
			row_width = 0;
			row_height = 0;
		}
		row_width += face->glyph->bitmap.width + 1;
		row_height = std::max(row_height, face->glyph->bitmap.rows);
	}
	width = std::max(width, row_width);
	height += row_height;


	std::vector<uint8_t> texture_atlas_data(width * height);

	int origin_x = 0;
	int origin_y = 0;
	row_height = 0;

	//Load glyphs onto a texture atlas
	for (size_t i = 0; i < MAX_CHARACTER_COUNT; ++i)
	{
		FT_Error result = FT_Load_Char(face, i, FT_LOAD_RENDER);
		SK_ASSERT(!result, "FreeType: Couldn't load character: {0}, from file {1}, Error: {2}", i, path, FT_Error_String(result));
		
		if (origin_x + face->glyph->bitmap.width + 1 >= max_width)
		{
			origin_y += row_height;
			row_height = 0;
			origin_x = 0;
		}

		characters[i].advance.x = face->glyph->advance.x >> 6;
		characters[i].advance.y = face->glyph->advance.y >> 6;

		characters[i].size.x = face->glyph->bitmap.width;
		characters[i].size.y = face->glyph->bitmap.rows;

		characters[i].bearing.x = face->glyph->bitmap_left;
		characters[i].bearing.y = face->glyph->bitmap_top;

		characters[i].texture_coordinate.x = origin_x / (float)width;
		characters[i].texture_coordinate.y = origin_y / (float)height;

		std::memcpy(texture_atlas_data.data() + (origin_x + origin_y * width), face->glyph->bitmap.buffer, 
			face->glyph->bitmap.width * face->glyph->bitmap.rows);

		row_height = std::max(row_height, face->glyph->bitmap.rows);
		origin_x += face->glyph->bitmap.width + 1;
	}

	ImageProps texture_atlas_props{};
	texture_atlas_props.width = width;
	texture_atlas_props.height = height;
	texture_atlas_props.data = texture_atlas_data.data();
	texture_atlas_props.format = VK_FORMAT_R8_UNORM;
	texture_atlas = makeShared<Image>(texture_atlas_props);
}

Font::~Font()
{
	FT_Done_Face(face);
}
