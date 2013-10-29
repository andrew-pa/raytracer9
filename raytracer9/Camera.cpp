#include "Camera.h"

namespace raytracer9
{
	Camera::Camera(vec3 _p, vec3 _l, int _sw, int _sh)
		: pos(_p), sw(_sw), sh(_sh)
	{
		look = norm(_l - _p);
		static const vec3 down = vec3(0, -1, 0);
		right = 1.5 * norm(look.cross(down));
		up = 1.5 * norm(look.cross(right));
	}

	void Camera::lookAt(vec3 npos, vec3 target)
	{
		pos = npos;
		look = norm(target - npos);
		static const vec3 down = vec3(0, -1, 0);
		right = 1.5f * norm(look.cross(down));
		up = 1.5f * norm(look.cross(right));
	}
};