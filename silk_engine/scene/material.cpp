#include "material.h"

void Material::set(std::string_view name, const VkDescriptorBufferInfo& buffer_info)
{
	if (auto resource = pipeline->getShader()->getIfExists(name))
		descriptor_sets[resource->set].setBufferInfo(resource->binding, { buffer_info });
}

void Material::set(std::string_view name, const VkDescriptorImageInfo& image_info)
{
	if (auto resource = pipeline->getShader()->getIfExists(name))
		descriptor_sets[resource->set].setImageInfo(resource->binding, { image_info });
}

void Material::bind()
{
	pipeline->bind();
	for (auto&& [set, descriptor] : descriptor_sets)
		descriptor.bind(set);
}
