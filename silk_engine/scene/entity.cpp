#include "entity.h"
#include "scene.h"

Entity::Entity(entt::entity handle, Scene* scene)
	: entity(handle), scene(scene)
{
}