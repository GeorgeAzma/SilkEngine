#pragma once

#include "scene.h"

class Entity : NonCopyable
{
public:
	Entity(entt::entity handle, Scene* scene);

	template<typename T, typename... Args>
	T& add(Args&&... args)
	{
		SK_VERIFY(!has<T>(), "Entity already has specified component");
		return scene->registry.emplace<T>(entity, std::forward<Args>(args)...);
	}

	template<typename T>
	bool has() const
	{
		return scene->registry.any_of<T>(entity);
	}

	template<typename T>
	void remove() const
	{
		SK_VERIFY(has<T>(), "Entity doesn't have specified component");
		scene->registry.remove<T>(entity);
	}

	template<typename T>
	T& get()
	{
		SK_VERIFY(has<T>(), "Entity doesn't have specified component");
		return scene->registry.get<T>(entity);
	}

	//Function signature should be: void(Component&)
	template<typename T, typename... Fn>
	void update(Fn&&... func)
	{
		scene->registry.patch<T>(entity, std::forward<Fn>(func)...);
	}

	template<typename T, typename... Args>
	void replace(Args&&... args)
	{
		scene->registry.replace<T>(entity, std::forward<Args>(args)...);
	}

	operator entt::entity () const { return entity; }
	operator bool() const { return entity != entt::null; }
	bool operator==(const Entity& other) const { return entity == other.entity && scene == other.scene; }

private:
	entt::entity entity = entt::null;
	Scene* scene = nullptr;
	friend class Scene;
};

struct ScriptableEntity
{
public:
	template<typename T>
	T& get() { return entity->get<T>(); }

protected:
	virtual void onCreate() = 0;
	virtual void onUpdate() = 0;
	virtual void onDestroy() = 0;

public:
	shared<Entity> entity = nullptr;
	friend class Scene;
};