#pragma once

using TypeId = std::size_t;

template<typename T>
class TypeInfo {
public:
	TypeInfo() = delete;

	/**
	 * Get the type ID of K which is a base of T.
	 * @tparam K The type ID K.
	 * @return The type ID.
	 */
	template<typename K,
		typename = std::enable_if_t<std::is_convertible_v<K*, T*>>>
		static TypeId getTypeId() noexcept 
	{
		std::type_index type_index(typeid(K));
		if (auto it = type_map.find(type_index); it != type_map.end())
			return it->second;
		const auto id = nextTypeId();
		type_map.emplace(type_index, id);
		return id;
	}

private:
	/**
	 * Get the next type ID for T
	 * @return The next type ID for T.
	 */
	static TypeId nextTypeId() noexcept 
	{
		const auto id = next_type_id;
		++next_type_id;
		return id;
	}

	// Next type ID for T.
	static TypeId next_type_id;
	static std::unordered_map<std::type_index, TypeId> type_map;
};

template<typename K>
TypeId TypeInfo<K>::next_type_id = 0;

template<typename K>
std::unordered_map<std::type_index, TypeId> TypeInfo<K>::type_map = {};