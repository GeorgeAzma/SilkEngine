#pragma once

#include "gfx/enums.h"
#include "gfx/descriptors/descriptor_set.h"
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

class Shader : NonCopyable
{
	struct Define
    {
        std::string name = "";
        std::string value = "";
    };

    struct Extension
    {
        std::string name = "";
        std::string behavior = "require";
    };

    class Includer : public glslang::TShader::Includer
    {
        IncludeResult* includeLocal(const char* header, const char* includer, size_t inclusion_depth) override;
        IncludeResult* includeSystem(const char* header, const char* includer, size_t inclusion_depth) override;
        void releaseInclude(IncludeResult* result) override;
    };

	class Uniform 
	{
		friend class Shader;

	public:
		explicit Uniform(int32_t binding = -1, int32_t offset = -1, int32_t size = -1, int32_t gl_type = -1, bool read_only = false, bool write_only = false, VkShaderStageFlags stage_flags = 0, int32_t count = -1)
			: binding(binding), offset(offset), size(size), gl_type(gl_type), read_only(read_only), write_only(write_only), stage_flags(stage_flags), count(count)
		{
		}

		int32_t getBinding() const { return binding; }
		int32_t getOffset() const { return offset; }
		int32_t getSize() const { return size; }
		int32_t getGlType() const { return gl_type; }
		int32_t getCount() const { return count; }
		bool isReadOnly() const { return read_only; }
		bool isWriteOnly() const { return write_only; }
		VkShaderStageFlags getStageFlags() const { return stage_flags; }

	private:
		int32_t binding;
		int32_t offset;
		int32_t size;
		int32_t gl_type;
		int32_t count;
		bool read_only;
		bool write_only;
		VkShaderStageFlags stage_flags;
	};

	class UniformBlock 
    {
		friend class Shader;

	public:
		enum class Type { None, Uniform, Storage, Push };

		explicit UniformBlock(int32_t binding = -1, int32_t size = -1, VkShaderStageFlags stage_flags = 0, Type type = Type::Uniform) 
            : binding(binding), size(size), stage_flags(stage_flags), type(type)
        {
		}

		int32_t getBinding() const { return binding; }
		int32_t getSize() const { return size; }
		VkShaderStageFlags getStageFlags() const { return stage_flags; }
		Type getType() const { return type; }
		const std::unordered_map<std::string, Uniform>& getUniforms() const { return uniforms; }

		std::optional<Uniform> getUniform(const std::string& name) const 
		{
			auto it = uniforms.find(name);

			if (it == uniforms.end())
				return std::nullopt;

			return it->second;
		}

	private:
		int32_t binding;
		int32_t size;
		VkShaderStageFlags stage_flags;
		Type type;
		std::unordered_map<std::string, Uniform> uniforms;
	};

	class Attribute 
	{
		friend class Shader;

	public:
		explicit Attribute(int32_t set = -1, int32_t location = -1, int32_t size = -1, int32_t gl_type = -1) 
			: set(set), location(location), size(size), gl_type(gl_type) 
		{
		}

		int32_t getSet() const { return set; }
		int32_t getLocation() const { return location; }
		int32_t getSize() const { return size; }
		int32_t getGlType() const { return gl_type; }

	private:
		int32_t set;
		int32_t location;
		int32_t size;
		int32_t gl_type;
	};

public:
	enum class Type : uint32_t
	{
		NONE = 0,
		VERTEX = 1,
		FRAGMENT = 2,
		GEOMETRY = 4,
		COMPUTE = 8,
		TESSELATION_CONTROL = 16,
		TESSELATION_EVALUATION = 32
	};

public:
    Shader(const std::string& file, const std::vector<Define>& defines = {});
	~Shader();

    void compile(const std::vector<Define>& defines = {});

	std::optional<uint32_t> getDescriptorLocation(const std::string& name) const;
	std::optional<uint32_t> getDescriptorSize(const std::string& name) const;
	std::optional<Uniform> getUniform(const std::string& name) const;
	std::optional<UniformBlock> getUniformBlock(const std::string& name) const;
	std::optional<Attribute> getAttribute(const std::string& name) const;
	std::optional<VkDescriptorType> getDescriptorType(uint32_t location) const;
	const std::vector<VkPushConstantRange>& getPushConstantRanges() const { return push_constant_ranges;  }
	shared<DescriptorSet> getDescriptorSet() { return descriptor_set->getWrites().empty() ? nullptr : descriptor_set; }
	const std::vector<VkPipelineShaderStageCreateInfo>& getPipelineShaderStageInfos() const { return pipeline_shader_stage_infos; }

public:
    static EShLanguage getEshLanguage(Type shader_type);
    static VkShaderStageFlagBits getVulkanType(Type shader_type);
    static Type getStringType(const std::string& shader_string);
    static std::string getTypeFileExtension(Type shader_type);
    static glslang::EShTargetClientVersion getEshClientVersion(APIVersion api_version);
    static TBuiltInResource getResources();
	static int32_t computeSize(const glslang::TType* ttype);

private:
	std::unordered_map<uint32_t, std::string> parse(const std::string& file);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& source) const;
    
	//Reflection
	void loadUniformBlock(const glslang::TProgram& program, VkShaderStageFlagBits stage_flag, int32_t index);
	void loadUniform(const glslang::TProgram& program, VkShaderStageFlagBits stage_flag, int32_t index);
	void loadAttribute(const glslang::TProgram& program, int32_t index);

private:
    std::string file;
    std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stage_infos;
	
	//Reflection data
	std::optional<uint32_t> local_sizes[3]; 
	std::vector<VkPushConstantRange> push_constant_ranges;
	std::unordered_map<std::string, uint32_t> descriptor_locations;
	std::unordered_map<std::string, uint32_t> descriptor_sizes;
	std::unordered_map<uint32_t, VkDescriptorType> descriptor_types;
	std::unordered_map<std::string, UniformBlock> uniform_blocks;
	std::unordered_map<std::string, Uniform> uniforms;
	std::unordered_map<std::string, Attribute> attributes;
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
	shared<DescriptorSet> descriptor_set = nullptr;
};