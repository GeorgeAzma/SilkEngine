#pragma once

#include "mesh2D.h"
#include "gfx/ui/font.h"

class TextMesh : public Mesh2D
{
public:
	TextMesh(std::string_view text, uint32_t size = 32, shared<Font> font = nullptr);
};