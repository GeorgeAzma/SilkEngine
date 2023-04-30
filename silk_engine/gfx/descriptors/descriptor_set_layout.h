#pragma once

struct Hash;
struct Equal;

class DescriptorSetLayout : NoCopy
{
public:
	DescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~DescriptorSetLayout();

	const std::map<uint32_t, VkDescriptorSetLayoutBinding>& getBindings() const { return bindings; }
	uint32_t getDynamicDescriptorCount() const { return dynamic_descriptor_count; }
	operator const VkDescriptorSetLayout& () const { return layout; }

private:
	VkDescriptorSetLayout layout = nullptr;
	std::map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
	uint32_t dynamic_descriptor_count = 0;

public:
	static shared<DescriptorSetLayout> get(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		if (auto layout = descriptor_set_layouts.find(bindings); layout != descriptor_set_layouts.end())
			return layout->second;
		return add(bindings);
	}
	static shared<DescriptorSetLayout> add(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		return descriptor_set_layouts.insert_or_assign(bindings, makeShared<DescriptorSetLayout>(bindings)).first->second;
	}
	static void destroy() { descriptor_set_layouts.clear(); }

private:
	struct Hash
	{
		size_t operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings) const
		{
			size_t result = bindings.size();
			for (const VkDescriptorSetLayoutBinding& b : bindings)
				result ^= b.binding ^ (b.descriptorType << 8) ^ (b.descriptorCount << 16) ^ (b.stageFlags << 24);
			return result;
		}
	};

	struct Equal
	{
		bool operator()(const std::vector<VkDescriptorSetLayoutBinding>& bindings, const std::vector<VkDescriptorSetLayoutBinding>& other) const
		{
			if (other.size() != bindings.size())
				return false;

			for (size_t i = 0; i < bindings.size(); i++)
			{
				if (other[i].binding != bindings[i].binding ||
					other[i].descriptorType != bindings[i].descriptorType ||
					other[i].descriptorCount != bindings[i].descriptorCount ||
					other[i].stageFlags != bindings[i].stageFlags)
					return false;
			}

			return true;
		}
	};

	static inline std::unordered_map<std::vector<VkDescriptorSetLayoutBinding>, shared<DescriptorSetLayout>, Hash, Equal> descriptor_set_layouts{};
};