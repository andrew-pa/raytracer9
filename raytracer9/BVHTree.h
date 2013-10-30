#pragma once
#include "Math.h"
#include "Primitive.h"

namespace raytracer9
{
	class BVHNode :
		public Primitive
	{
	public:
		BVHNode();
		~BVHNode();
	};
}

