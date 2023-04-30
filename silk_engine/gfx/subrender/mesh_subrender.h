#pragma once

#include "subrender.h"

class RenderPass;

class MeshSubrender : public Subrender
{
public:
	MeshSubrender(RenderPass& render_pass, uint32_t subpass = 0);

	void render() override;
};