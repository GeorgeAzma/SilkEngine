#pragma once

class RawModel;
class Mesh;
class Image;

class Model
{
public:
	Model(const path& file);
	Model(const RawModel& raw_model);

	static RawModel load(const path& file);

	const std::vector<shared<Mesh>>& getMeshes() const;
	const std::vector<std::vector<shared<Image>>>& getImages() const;

private:
	std::vector<shared<Mesh>> meshes;
	std::vector<std::vector<shared<Image>>> images;
	path file;

public:
	static shared<Model> get(std::string_view name) 
	{ 
		if (auto it = models.find(name); it != models.end()) 
			return it->second; 
		return nullptr; 
	}
	static shared<Model> add(std::string_view name, const shared<Model>& model) { return models.insert_or_assign(name, model).first->second; }
	static void destroy() { models.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Model>> models{};
};