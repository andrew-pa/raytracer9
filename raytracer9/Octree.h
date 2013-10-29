#pragma once
#include "stdinc.h"
#include "Math.h"
#include "Primitive.h"

namespace raytracer9
{
	aabb* SubdivideAABB(aabb box);

	class OctreeNode :
		public Primitive
	{
		aabb _bounds;
		vector<Primitive*> nodes;
	public:
		OctreeNode() { }
		
		static const int MAX_RECURSION = 10;

		OctreeNode(aabb bnds, const vector<Primitive*>& prims, int depth = MAX_RECURSION, uint targetPrims = 4);
		
		bool hit(const ray& r, hitrecord& hr) override;

		inline aabb bounds() const override { return _bounds; } 
		inline vec3 center() override { return _bounds.Center(); }
		inline bool containedBy(const aabb& b) override { return _bounds.Inside(b); }
	};
};

