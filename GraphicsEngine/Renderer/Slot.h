#pragma once

/// <summary>
/// Common.hlsli¿« register
/// </summary>


enum class Slot_B
{
	Camera = 0,
	Transform,
	Material,
	LightArray,
	MatrixPallete,
	IBL,
	End
};

enum class Slot_T
{
	Albedo = 0,
	Normal, 
	Position, 
	Depth, 
	Metalic, 
	Roughness, 
	AO, 
	Emissive, 
	Opacity,
	LightMap,
	GBuffer,
	Irradiance,
	End,
};


enum class Slot_S
{
	Linear = 0,

	End
};
