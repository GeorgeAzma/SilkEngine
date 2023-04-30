#pragma once

#include "subrender.h"

class Material;
class RenderPass;

class PostProcessSubrender : public Subrender
{
public:
	PostProcessSubrender(RenderPass& render_pass, uint32_t subpass = 0);

	void render() override;

private:
	shared<Material> material = nullptr;
};