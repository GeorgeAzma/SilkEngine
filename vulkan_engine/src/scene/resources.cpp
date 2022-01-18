#include "resources.h"
#include "meshes/circle_mesh.h"

void Resources::init()
{
	addMesh("Circle", std::make_shared<CircleMesh>());
	//addMaterial("3D", std::make_shared<Material>());
}

void Resources::cleanup()
{
	meshes.clear();
	materials.clear();
}

std::shared_ptr<Mesh> Resources::getMesh(const std::string& name)
{
	return meshes.at(name);
}


void Resources::addMesh(const std::string& name, std::shared_ptr<Mesh> mesh)
{
	mesh->name = name;
	meshes[name] = mesh;
}

void Resources::addMaterial(const std::string& name, std::shared_ptr<Material> material)
{
	materials[name] = material;
}
