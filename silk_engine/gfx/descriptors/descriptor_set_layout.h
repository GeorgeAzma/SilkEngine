#pragma once

struct Hash;
struct Equal;

class DescriptorSetLayout : NonCopyable
{
	friend class DescriptorSet;

public:
	~DescriptorSetLayout();

	DescriptorSetLayout& addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, size_t count = 1);
	void build();

	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return bindings_vector; }
	operator const VkDescriptorSetLayout& () const { return layout; }

private:
	VkDescriptorSetLayout layout = nullptr;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorSetLayoutBinding> bindings_vector;

public:
	static shared<DescriptorSetLayout> get(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	static shared<DescriptorSetLayout> add(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
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