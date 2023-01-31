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
};