#pragma once

#include "raw_mesh.h"

class Font;

struct TextMesh : public RawMesh2D
{
	TextMesh(std::string_view text, const shared<Font>& font = nullptr);
};