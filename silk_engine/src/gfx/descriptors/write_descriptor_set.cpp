#include "write_descriptor_set.h"

WriteDescriptorSet::WriteDescriptorSet(const WriteDescriptorSetProps& write_descriptor_set_props, const std::vector<VkDescriptorImageInfo>& image_infos)
	: image_infos(image_infos)
{
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.descriptorType = write_descriptor_set_props.descriptor_type;
	write_descriptor_set.descriptorCount = this->image_infos.size();
	write_descriptor_set.pImageInfo = this->image_infos.data();
	write_descriptor_set.pBufferInfo = nullptr;
	write_descriptor_set.dstBinding = write_descriptor_set_props.dst_binding;
	write_descriptor_set.dstArrayElement = write_descriptor_set_props.dst_array_element;
}

WriteDescriptorSet::WriteDescriptorSet(const WriteDescriptorSetProps& write_descriptor_set_props, const std::vector<VkDescriptorBufferInfo>& buffer_infos)
	: buffer_infos(buffer_infos)
{
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.descriptorType = write_descriptor_set_props.descriptor_type;
	write_descriptor_set.descriptorCount = this->buffer_infos.size();
	write_descriptor_set.pImageInfo = nullptr;
	write_descriptor_set.pBufferInfo = this->buffer_infos.data();
	write_descriptor_set.dstBinding = write_descriptor_set_props.dst_binding;
	write_descriptor_set.dstArrayElement = write_descriptor_set_props.dst_array_element;
}

