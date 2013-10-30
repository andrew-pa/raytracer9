#pragma once
#include "stdinc.h"
#include "Math.h"
#include "Primitive.h"

namespace raytracer9
{
	class BVHNode :
		public Primitive
	{
		Primitive* _left, *_right;
		aabb _bounds;
	public:
		BVHNode(Primitive* l, Primitive* r)
			: _left(l), _right(r), _bounds(l->bounds(), r->bounds()) { }
		BVHNode(vector<Primitive*>& objs, Axis axi = Axis::X);
		bool hit(const ray& r, hitrecord& hr) override
		{
			return false;
		}

		aabb bounds() const override
		{
			return _bounds;
		}
		inline const Primitive* left() const { return _left;  }
		inline const Primitive* right() const { return _right; }
	};
}

