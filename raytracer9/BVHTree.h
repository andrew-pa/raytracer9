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
		bool hit(const ray& r, hitrecord& hr) override;

		aabb bounds() const override
		{
			return _bounds;
		}
		inline const Primitive* left() const { return _left;  }
		inline const Primitive* right() const { return _right; }

		inline material*& mat() { throw exception("BVHTree has no material"); }

		friend bool bvh_tree_hit_stack(BVHNode* root, const ray& r, hitrecord& hr);
	};

	bool bvh_tree_hit_stack(BVHNode* root, const ray& r, hitrecord& hr);
}

