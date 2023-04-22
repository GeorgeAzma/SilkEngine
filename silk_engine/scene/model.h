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
	static shared<Model> get(std::string_view name) { return models.at(name); }
	static void add(std::string_view name, const shared<Model> model) { models.insert_or_assign(name, model); }
	static void destroy() { models.clear(); }

private:
	static inline std::unordered_map<std::string_view, shared<Model>> models{};
};