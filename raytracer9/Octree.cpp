#include "Octree.h"

namespace raytracer9
{
	aabb* SubdivideAABB(aabb box)
	{
		aabb* boxes = new aabb[8];
		static const vec3 bot[8] =
		{                  
			vec3(-0.5, -0.5, -0.5),
			vec3(+0.5, -0.5, -0.5),
			vec3(-0.5, +0.5, -0.5),
			vec3(+0.5, +0.5, -0.5),
			vec3(-0.5, -0.5, +0.5),
			vec3(+0.5, -0.5, +0.5),
			vec3(-0.5, +0.5, +0.5),
			vec3(+0.5, +0.5, +0.5)

		};
		for(int i = 0; i < 8; ++i)
		{
			vec3 of = bot[i] * box.Extent();
			boxes[i] = aabb(box.Center()+of, box.Extent()*.5f);
		}
		return boxes;
	}

	OctreeNode::OctreeNode(aabb b, const vector<Primitive*>& prims, int depth, uint targetPrims)
	{
		_bounds = b;
		if (depth <= 0 || prims.size() <= targetPrims)
		{
			nodes = prims;
			//cout << "[L " << prims.size() << "]";
			return;
		}
		//nodes = prims;
		vector<Primitive*> n;
		for(uint i = 0; i < prims.size(); ++i)
		{
			if (prims[i]->containedBy(_bounds) || prims[i]->intersects(_bounds))
			{
				n.push_back(prims[i]);
			}
		}
		if(n.size() <= targetPrims)
		{
			nodes = n;
			//cout << "[LX " << n.size() << "]";
		}
		else
		{
			aabb* chb = SubdivideAABB(b);
			//cout << "[N " << endl;
			for(int i = 0; i < 8; ++i)
			{
				nodes.push_back(new OctreeNode(chb[i], n, depth-1, targetPrims));
			}
			//cout << "]" << endl;
		}
	}

	bool OctreeNode::hit(const ray& r, hitrecord& hr)
	{
		if(!_bounds.Hit(r)) return false;
		if(nodes.size() > 0)
		{
			bool hit = false;
			for(auto ob = nodes.begin(); ob != nodes.end(); ++ob)
			{
				hitrecord nhr;
				nhr.t = hr.t;
				if((*ob)->hit(r, nhr)) 
				{
					if(nhr.t < hr.t)
					{
						hit = true;
						hr = nhr;
					}
				}
			}
			return hit;
		}
		return false;
	}
};
