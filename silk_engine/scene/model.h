#pragma once

class RawModel;
class Mesh;
class Image;

class Model : NoCopy
{
public:
	Model(const fs::path& file);
	Model(const RawModel& raw_model);

	static RawModel load(const fs::path& file);

	const std::vector<std::vector<shared<Image>>>& getImages() const;

private:
	shared<Mesh> mesh;
	std::vector<std::vector<shared<Image>>> images;
	fs::path file;

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