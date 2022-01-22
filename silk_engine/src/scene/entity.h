#pragma once

#include "scene.h"

class Entity
{
public:
	Entity(entt::entity handle, Scene* scene);

	~Entity()
	{
	}

	template<typename T, typename... Args>
	T& addComponent(Args&&... args)
	{
		SK_ASSERT(!hasComponent<T>(), "Entity already has specified component");
		return scene->registry.emplace<T>(entity, std::forward<Args>(args)...);
	}

	template<typename T>
	bool hasComponent() const
	{
		return scene->registry.any_of<T>(entity);
	}

	template<typename T>
	void removeComponent() const
	{
		SK_ASSERT(hasComponent<T>(), "Entity doesn't have specified component");
		scene->registry.remove<T>(entity);
	}

	template<typename T>
	T& getComponent()
	{
		SK_ASSERT(hasComponent<T>(), "Entity doesn't have specified component");
		return scene->registry.get<T>(entity);
	}

private:
	operator entt::entity () const { return entity; }

private:
	entt::entity entity = entt::null;
	Scene* scene = nullptr;
	friend class Scene;
};

struct ScriptableEntity
{
public:
	template<typename T>
	T& getComponent() { return entity->getComponent<T>(); }

protected:
	virtual void onCreate() = 0;
	virtual void onUpdate() = 0;
	virtual void onDestroy() = 0;

public:
	shared<Entity> entity = nullptr;
	friend class Scene;
};