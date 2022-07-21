#pragma once

class RawModel;
class Mesh;
class Image2D;

class Model
{
public:
	Model(std::string_view file);
	Model(const RawModel& raw_model);

	static RawModel load(std::string_view file);

	const std::vector<shared<Mesh>>& getMeshes() const;
	const std::vector<std::vector<shared<Image2D>>>& getImages() const;
	std::string_view getPath() const { return path; }

private:
	std::vector<shared<Mesh>> meshes;
	std::vector<std::vector<shared<Image2D>>> images;
	std::string path;
};