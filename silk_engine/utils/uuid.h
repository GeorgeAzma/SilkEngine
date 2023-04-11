#pragma once

struct UUID
{
	UUID() : uuid(RNG::Uint()) {}
	UUID(uint64_t uuid) : uuid(uuid) {}

	operator uint64_t () const { return uuid; }
	bool operator==(const UUID& other) const { return uuid == other.uuid; }

private:
	uint64_t uuid;
};

namespace std
{
	template<>
	struct hash<UUID>
	{
		size_t operator()(const UUID& uuid) const
		{
			return uuid;
		}
	};
}