#pragma once

enum class Type
{
	BOOL,
	BYTE,
	UBYTE,
	SHORT,
	USHORT,
	INT,
	UINT,
	FLOAT,
	DOUBLE,
	VEC2,
	VEC3,
	VEC4,
	VEC2I,
	VEC3I,
	VEC4I,
	VEC2U,
	VEC3U,
	VEC4U,
	VEC2D,
	VEC3D,
	VEC4D,
	MAT2,
	MAT3,
	MAT4,
	MAT2I,
	MAT3I,
	MAT4I,
	MAT2U,
	MAT3U,
	MAT4U,
	MAT2D,
	MAT3D,
	MAT4D
};

class EnumInfo
{
public:
	static void type(Type type);
};