#include "material.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/images/image.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/pipeline/compute_pipeline.h"
#include "silk_engine/gfx/descriptors/descriptor_set.h"

Material::Material(const shared<GraphicsPipeline>& graphics_pipeline)
	: pipeline(graphics_pipeline) {}

Material::Material(const shared<ComputePipeline>& compute_pipeline)
	: pipeline(compute_pipeline) {}

void Material::set(std::string_view name, const Buffer& buffer)
{
	set(name, buffer.getDescriptorInfo());
}

void Material::set(std::string_view name, const std::vector<shared<Buffer>>& buffers)
{
	std::vector<VkDescriptorBufferInfo> buffer_infos(buffers.size());
	for (size_t i = 0; i < buffers.size(); ++i)
		buffer_infos[i] = buffers[i]->getDescriptorInfo();
	set(name, buffer_infos);
}

void Material::set(std::string_view name, const VkDescriptorBufferInfo& buffer)
{
	if (const Shader::ResourceLocation& location = pipeline->getShader()->getLocation(name))
		getDescriptorSet(location.set)->write(location.binding, buffer);
}

void Material::set(std::string_view name, const std::vector<VkDescriptorBufferInfo>& buffers)
{
	if (const Shader::ResourceLocation& location = pipeline->getShader()->getLocation(name))
		getDescriptorSet(location.set)->write(location.binding, buffers);
}

void Material::set(std::string_view name, const Image& image)
{
	set(name, image.getDescriptorInfo());
}

void Material::set(std::string_view name, const std::vector<shared<Image>>& images)
{
	std::vector<VkDescriptorImageInfo> image_infos(images.size());
	for (size_t i = 0; i < images.size(); ++i)
		image_infos[i] = images[i]->getDescriptorInfo();
	set(name, image_infos);
}

void Material::set(std::string_view name, const VkDescriptorImageInfo& image)
{
	if (const Shader::ResourceLocation& location = pipeline->getShader()->getLocation(name))
		getDescriptorSet(location.set)->write(location.binding, image);
}

void Material::set(std::string_view name, const std::vector<VkDescriptorImageInfo>& images)
{
	if (const Shader::ResourceLocation& location = pipeline->getShader()->getLocation(name))
		getDescriptorSet(location.set)->write(location.binding, images);
}

void Material::set(std::string_view name, VkBufferView buffer_view)
{
	if (const Shader::ResourceLocation& location = pipeline->getShader()->getLocation(name))
		getDescriptorSet(location.set)->write(location.binding, buffer_view);
}

void Material::set(std::string_view name, const std::vector<VkBufferView>& buffer_views)
{
	if (const Shader::ResourceLocation& location = pipeline->getShader()->getLocation(name))
		getDescriptorSet(location.set)->write(location.binding, buffer_views);
}

void Material::bind()
{
	pipeline->bind();
	for (auto&& [set, descriptor_set] : descriptor_sets)
		descriptor_set->bind(set);
}

const shared<DescriptorSet>& Material::getDescriptorSet(uint32_t set)
{
	if (auto it = descriptor_sets.find(set); it != descriptor_sets.end())
		return it->second;
	return descriptor_sets.emplace(set, makeShared<DescriptorSet>(*pipeline->getShader()->getReflectionData().descriptor_set_layouts.at(set))).first->second;
}