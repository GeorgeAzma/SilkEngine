#include "material.h"

void Material::bind()
{
	pipeline->bind();
	for (auto&& [set, descriptor] : descriptor_sets)
		descriptor.bind(set);
}
