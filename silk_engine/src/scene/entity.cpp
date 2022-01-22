#include "entity.h"

Entity::Entity(entt::entity handle, Scene* scene)
	: entity(handle), scene(scene)
{
}

Entity::~Entity()
{
	if(entity != entt::null)
		scene->removeEntity(entity);
}