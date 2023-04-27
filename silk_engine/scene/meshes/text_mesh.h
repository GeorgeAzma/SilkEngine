#pragma once

#include "raw_mesh.h"

class Font;

struct TextMesh : public RawMesh2D
{
	TextMesh(std::string_view text, uint32_t size = 32, shared<Font> font = nullptr);
};