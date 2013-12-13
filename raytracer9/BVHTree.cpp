#include "BVHTree.h"
#include <iostream>
#include <fstream>

namespace raytracer9
{
	BVHNode::BVHNode(vector<Primitive*>& objs, Axis axi)
	{
		//static ofstream log = ofstream("bvh_log.txt");
		if (objs.size() == 0)
		{
			_left = nullptr; _right = nullptr;
			//log << "[0]" << endl;
			_bounds = aabb(vec3(), 0);
		}
		else if (objs.size() == 1)
		{
			_left = objs[0]; _right = nullptr;
			//log << "[1 (" << hex << (uint)_left << ")]" << dec << endl;
			_bounds = _left->bounds();
		}
		else if (objs.size() == 2)
		{
			_left = objs[0]; _right = objs[1];
			//log << "[2 (" << hex << (uint)_left << "), (" << (uint)_right << ")]" << dec << endl;
			_bounds = aabb(_left->bounds(), _right->bounds());
		}
		else
		{
			sort(objs.begin(), objs.end(), [&](Primitive* a, Primitive* b)
			{
				float pac = a->center()[axi];
				float pbc = b->center()[axi];
				return pac > pbc;
			});
			auto half = objs.size() / 2;
			auto left_half = vector<Primitive*>(objs.begin(), objs.begin()+half);
			auto right_half = vector<Primitive*>(objs.begin() + half, objs.end());
			//log << "[" << objs.size() << " " << half << " " << left_half.size() << ", " << right_half.size() << "]" << endl;
			//log << "[N (" << left_half.size() << ":" << right_half.size() << ") " << endl << "\t";
			_left = new BVHNode(left_half, (Axis)( ((uint)axi + 1) % 3 ));
			_right = new BVHNode(right_half, (Axis)(((uint)axi + 1) % 3));
			//log << "\t" << endl << "]";
			_bounds = aabb(_left->bounds(), _right->bounds());
			//cout << _bounds.Area() << endl;
		}
	}

	bool BVHNode::hit(const ray& r, hitrecord& hr)
	{
		if (!_bounds.Hit(r)) return false;
		hitrecord lhr, rhr;
		lhr.t = hr.t;
		rhr.t = hr.t;
		bool lh = (_left != nullptr ? _left->hit(r, lhr) : false);
		bool rh = (_right != nullptr ? _right->hit(r, rhr) : false);
		if (lh && rh)
		{
			if (lhr.t < rhr.t)
			{
				hr = lhr;
				return true;
			}
			else
			{
				hr = rhr;
				return true;
			}
		}
		else
		{
			if (lh && !rh)
			{
				hr = lhr;
				return true;
			}
			else if(!lh && rh)
			{
				hr = rhr;
				return true;
			}
		}
		return false;
	}


	bool bvh_tree_hit_stack(BVHNode* root, const ray& r, hitrecord& hr)
	{
		const uint stack_size = 64;
#define check_ptr { if(stk_ptr > stack_size) stk_ptr = 0; if(stk_ptr < 0) stk_ptr = stack_size-1; }
		bool rtv = false;
		Primitive* stk[stack_size];
		int stk_ptr = stack_size-1;
		stk[stk_ptr] = root;
		stk_ptr--; check_ptr
		while (stk_ptr != stack_size-1)//(!stk.empty())
		{
			Primitive* pr = stk[stk_ptr+1];//stk.top();
			auto n = dynamic_cast<BVHNode*>(pr);
			if (n != nullptr)
			{
				stk_ptr++; check_ptr//stk.pop();
				stk[stk_ptr] = n->_right; stk_ptr--; check_ptr//stk.push(n->_right);
				stk[stk_ptr] = n->_left; stk_ptr--; check_ptr//stk.push(n->_left);
			}
			else
			{
				stk_ptr++; check_ptr//stk.pop();
				hitrecord nhr;
				nhr.t = hr.t;
				if (pr->hit(r, nhr))
				{
					hr = nhr;
					return true;
				}
			}
		}
		return rtv;
	}
};

