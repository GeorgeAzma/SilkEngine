#include "font.h"
#include "silk_engine/gfx/images/raw_image.h"

void Font::init()
{
	FT_Error result = FT_Init_FreeType(&free_type_library);
	SK_VERIFY(!result, "FreeType: Couldn't initialize free type");
}

void Font::destroy()
{
	FT_Done_FreeType(free_type_library); 
	fonts.clear();
}

Font::Font(const fs::path& file, uint32_t size)
	: characters(MAX_CHARACTER_COUNT), size(size), file(file)
{
	FT_Error result = FT_New_Face(free_type_library, this->file.string().c_str(), 0, &face);
	SK_VERIFY(!result, "FreeType: Couldn't create new face: {}", this->file.string());

	FT_Set_Pixel_Sizes(face, 0, size);
	
	//Step 1: Get texture atlas dimensions
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t row_width = 0;
	uint32_t row_height = 0;

	uint32_t padding = 1; //Slight padding for avoiding resizing artifacts

	//This is just random assumption to make aspect ratio of texture atlas as close to 1 as possible
	const uint max_width = (size + padding) * sqrt(MAX_CHARACTER_COUNT); 
	SK_VERIFY(max_width > 0, "FreeType: Invalid texture input, it is too small");

	for (size_t i = 0; i < MAX_CHARACTER_COUNT; ++i)
	{
		FT_Error result = FT_Load_Char(face, i, FT_LOAD_RENDER);
		SK_VERIFY(!result, "FreeType: Couldn't load character: {}, from file {}", i, file);
		if (row_width + face->glyph->bitmap.width + 1 + padding >= max_width)
		{
			width = std::max(width, row_width);
			height += row_height;
			row_width = 0;
			row_height = 0;
		}
		row_width += face->glyph->bitmap.width + 1 + padding;
		row_height = std::max(row_height, face->glyph->bitmap.rows + padding);
	}
	width = std::max(width, row_width);
	height += row_height;


	//Step 2: Load glyphs onto a texture atlas
	RawImage texture_atlas_bitmap{};
	texture_atlas_bitmap.width = width;
	texture_atlas_bitmap.height = height;
	texture_atlas_bitmap.channels = 4;
	texture_atlas_bitmap.allocate();

	int origin_x = 0;
	int origin_y = 0;
	row_height = 0;

	for (size_t i = 0; i < MAX_CHARACTER_COUNT; ++i)
	{
		FT_Error result = FT_Load_Char(face, i, FT_LOAD_RENDER);
		SK_VERIFY(!result, "FreeType: Couldn't load character: {}, from file {}, Error: {}", i, file, FT_Error_String(result));
		
		if (origin_x + face->glyph->bitmap.width + 1 + padding >= max_width)
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
		characters[i].texture_coordinate.z = (origin_x + face->glyph->bitmap.width) / (float)width;
		characters[i].texture_coordinate.w = (origin_y + face->glyph->bitmap.rows) / (float)height;		

		for (size_t j = 0; j < face->glyph->bitmap.rows; ++j)
		{
			for (size_t k = 0; k < face->glyph->bitmap.width; ++k)
			{
				size_t off = (origin_x + k + (j + origin_y) * width) * 4;
				texture_atlas_bitmap.pixels[off + 0] = 255;
				texture_atlas_bitmap.pixels[off + 1] = 255;
				texture_atlas_bitmap.pixels[off + 2] = 255;
				texture_atlas_bitmap.pixels[off + 3] = face->glyph->bitmap.buffer[j * face->glyph->bitmap.width + k];
			}
		}

		row_height = std::max(row_height, face->glyph->bitmap.rows + padding);
		origin_x += face->glyph->bitmap.width + 1 + padding;
	}

	Image::Props texture_atlas_props{};
	texture_atlas_props.width = width;
	texture_atlas_props.height = height;
	texture_atlas = makeShared<Image>(texture_atlas_props);
	texture_atlas->setData(texture_atlas_bitmap.pixels.data());
	texture_atlas->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	FT_Done_Face(face);
}

Font::~Font()
{
}

std::vector<Font::Character> Font::getCharacterLayout(std::string_view str)
{
	const int32_t new_line_offset = size / 8;
	const int32_t new_line_indent = size + new_line_offset;
	const int32_t tab_indent = size * 4;
	
	std::vector<Character> character_layout;
	character_layout.reserve(str.size());

	int32_t x = 0;
	int32_t y = 0;

	for (uchar c : str)
	{
		switch (c)
		{
		case '\n':
			y += new_line_indent;
			x = 0;
			continue;

		case '\t':
			x += tab_indent;
			continue;
		}

		int32_t char_x = x + this->characters[c].bearing.x;
		int32_t char_y = y - (this->characters[c].size.y - this->characters[c].bearing.y);

		character_layout.emplace_back
		(
			vec4(char_x, char_y, char_x + this->characters[c].size.x, char_y + this->characters[c].size.y) / vec4(size),
			this->characters[c].texture_coordinate
		);

		x += this->characters[c].advance.x;
	}

	return character_layout;
}