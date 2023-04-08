#pragma once

class Mesh;
class Model;
class Font;
class GraphicsPipeline;
class ComputePipeline;
class ThreadPool;
class Image;
class Shader;

class Resources
{
public:
	static void init();
	static void destroy();

	template<typename T>
	static shared<T> get(std::string_view name);

	template<typename T>
	static void add(std::string_view name, const shared<T>& t);

	template<typename T>
	static void remove(std::string_view name);

	static void reloadShaders();

public:
	static ThreadPool pool;
	static inline shared<Image> white_image = nullptr;

private:
	template<typename T>
	static inline std::unordered_map<std::string_view, shared<T>> resources;
};