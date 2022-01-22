#include "entity.h"

Entity::Entity(entt::entity handle, Scene* scene)
	: entity(handle), scene(scene)
{
}
