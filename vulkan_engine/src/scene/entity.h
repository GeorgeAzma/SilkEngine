#pragma once

#include "scene.h"

class Entity
{
public:
	Entity() = default;
	Entity(entt::entity handle, Scene* scene);

	template<typename T, typename... Args>
	T& addComponent(Args&&... args)
	{
		//VE_CORE_ASSERT(!hasComponent<T>(), "Entity already has specified component");
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
		VE_CORE_ASSERT(hasComponent<T>(), "Entity doesn't have specified component");
		scene->registry.remove<T>(entity);
	}

	template<typename T>
	T& getComponent()
	{
		VE_CORE_ASSERT(hasComponent<T>(), "Entity doesn't have specified component");
		return scene->registry.get<T>(entity);
	}

private:
	entt::entity entity;
	Scene* scene = nullptr;
};

struct ScriptableEntity
{
public:
	template<typename T>
	T& getComponent() { return entity.getComponent<T>(); }
protected:
	virtual void onCreate() = 0;
	virtual void onUpdate() = 0;
	virtual void onDestroy() = 0;

public:
	Entity entity;
	friend class Scene;
};