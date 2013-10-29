#pragma once
#include "Math.h"

namespace raytracer9
{
	struct ILight
	{
		inline virtual vec3 GetPointOnLight() = 0;
		inline virtual vec3 Color() = 0;
	};

	struct PointLight : ILight
	{
		vec3 pos, color;

		PointLight(vec3 p, vec3 c)
			: pos(p), color(c) { }

		inline vec3 GetPointOnLight() override
		{
			return pos;
		}

		inline vec3 Color() override
		{
			return color;
		}
	};

	//Area light?
};