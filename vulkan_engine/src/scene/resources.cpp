#include "resources.h"
#include "meshes/circle_mesh.h"

void Resources::init()
{
	addMesh("Circle", std::make_shared<CircleMesh>());
}

void Resources::cleanup()
{
	meshes.clear();
}

std::shared_ptr<Mesh> Resources::getMesh(const char* name)
{
	return meshes.at(name);
}


void Resources::addMesh(const char* name, std::shared_ptr<Mesh> mesh)
{
	meshes[name] = mesh;
}
