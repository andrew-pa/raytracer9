#pragma once
#include "Math.h"

namespace raytracer9
{
	class Camera
	{
	public:
		vec3 pos;
		vec3 look;
		vec3 up;
		vec3 right;
		int sw, sh;

		Camera() { }
		Camera(vec3 _p, vec3 _l, int _sw, int _sh);
		
		void lookAt(vec3 npos, vec3 target);

		inline ray generateRay(float x, float y)
		{
			ray r(pos, vec3());
			r.e = pos;
			float u = (x - ((float)sw / 2.f)) / (2.f * sw);
			float v = -(y - ((float)sh / 2.f)) / (2.f * sh);
			r.d = norm(look + (u * right) + (v * up));
			return r;
		}
	};
};

