#pragma once

struct WriteDescriptorSetProps
{
    uint32_t            dst_binding = 0;
    uint32_t            dst_array_element = 0;
    VkDescriptorType    descriptor_type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
};

class WriteDescriptorSet : NonCopyable, NonMovable
{
public:
    WriteDescriptorSet(const WriteDescriptorSetProps& write_descriptor_set_props, const std::vector<VkDescriptorImageInfo>& image_infos);
    WriteDescriptorSet(const WriteDescriptorSetProps& write_descriptor_set_props, const std::vector<VkDescriptorBufferInfo>& buffer_infos);

    void setDstDescriptorSet(const VkDescriptorSet& dst_set) { write_descriptor_set.dstSet = dst_set; }

    operator const VkWriteDescriptorSet& () const { return write_descriptor_set; }
    const VkWriteDescriptorSet& getWrite() const { return write_descriptor_set; }
    void setImageInfos(const std::vector<VkDescriptorImageInfo>& image_infos) 
    { 
        SK_ASSERT(this->image_infos.size() == image_infos.size(), "Vulkan: Invalid image infos size");
        this->image_infos = image_infos; 
        write_descriptor_set.pImageInfo = this->image_infos.data();
    }
    void setBufferInfos(const std::vector<VkDescriptorBufferInfo>& buffer_infos) 
    { 
        SK_ASSERT(this->buffer_infos.size() == buffer_infos.size(), "Vulkan: Invalid buffer infos size");
        this->buffer_infos = buffer_infos; 
        write_descriptor_set.pBufferInfo = this->buffer_infos.data();
    }

private:
    VkWriteDescriptorSet write_descriptor_set{};
    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
};