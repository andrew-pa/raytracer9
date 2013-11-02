#include "TriangleMesh.h"
#include <hash_map>
#include <fstream>
#include <iomanip>

namespace raytracer9
{
	TriangleMesh::TriangleMesh(const vector<vertex>& v,
		const vector<uint>& i, const matrix4x4& w
#ifdef TRIMESH_TREE_OCTREE
		, int octree_depth, uint octree_primTarget
#endif
		)
		: vertices(v), indices(i), _world(w), _inv_world(matrix_inverse(w))
	{
		aabb bnds; 
		vector<Primitive*> ts;
		for(uint ix = 0; ix < indices.size(); ix+=3)
		{
			bnds.AddPoint(vertices[indices[ix]].pos);
			bnds.AddPoint(vertices[indices[ix+1]].pos);
			bnds.AddPoint(vertices[indices[ix+2]].pos);
			ts.push_back(new Triangle(indices[ix], indices[ix+1], indices[ix+2], this));
		}
#ifdef TRIMESH_TREE_BVH
		_tree = new BVHNode(ts);
#endif
#ifdef TRIMESH_TREE_OCTREE
		_tree = new OctreeNode(bnds, ts, octree_depth, octree_primTarget);
#endif
	}
	bool operator ==(const TriangleMesh::vertex& a, const TriangleMesh::vertex& b)
	{
		return a.pos.x == b.pos.x && a.pos.y == b.pos.y && a.pos.z == b.pos.z;	
	}

	inline vec3 readvec3(istream& i)
	{
		float x, y, z;
		i >> x >> y >> z;
		return vec3(x, y, z);
	}

	TriangleMesh::TriangleMesh(const string& obj_file, const matrix4x4& w
#ifdef TRIMESH_TREE_OCTREE
		,int octree_depth, uint octree_primTarget
#endif
		)
		: _world(w), _inv_world(matrix_inverse(w))
	{
#pragma region Load OBJ
		vector<vec3> poss;
		vector<vec3> norms;
		vector<vec2> texcords;
		
		ifstream inf(obj_file);
		char comm[256] = {0};

		while(inf)
		{
			inf >> comm;
			if(!inf) break;
			if(strcmp(comm, "#")==0) continue;
			else if(strcmp(comm, "v")==0)
				poss.push_back(readvec3(inf));
			else if(strcmp(comm, "vn")==0)
				norms.push_back(readvec3(inf));
			else if(strcmp(comm, "vt")==0)
			{
				float u, v;
				inf >> u >> v;
				texcords.push_back(vec2(u, v));
			}
			else if(strcmp(comm, "f")==0)
			{
				for(uint ifa = 0; ifa < 3; ++ifa)
				{
					vertex v;
					uint ip, in, it;
					inf >> ip;
					v.pos = poss[ip - 1];
					if('/' == inf.peek())
					{
						inf.ignore();
						if('/' != inf.peek())
						{
							inf >> it;
							v.texc = texcords[it - 1];
						}
						if('/' == inf.peek())
						{
							inf.ignore();
							inf >> in;
							v.norm = norms[in - 1];
						}
					}

					auto iv = find(vertices.begin(), vertices.end(), v);
					if(iv == vertices.end())
					{
						indices.push_back(vertices.size());
						vertices.push_back(v);
					}
					else
					{
						indices.push_back(distance(vertices.begin(), iv));
					}
				}
			}
		}
#pragma endregion/*
		ofstream dou("triaw.txt");
		dou.precision(7);
		for(auto i = poss.begin(); i != poss.end(); ++i)
			dou << "v " << i->x << " " << i->y << " " << i->z << endl;*/
		aabb bnds; 
		vector<Primitive*> ts;
		for(auto iv = vertices.begin(); iv != vertices.end(); ++iv)
			bnds.AddPoint(iv->pos);
		for(uint ix = 0; ix < indices.size(); ix+=3)
		{
			ts.push_back(new Triangle(indices[ix], indices[ix+1], indices[ix+2], this));
		}
#ifdef TRIMESH_TREE_BVH
		_tree = new BVHNode(ts);
#endif
#ifdef TRIMESH_TREE_OCTREE
		_tree = new OctreeNode(bnds, ts, octree_depth, octree_primTarget);
#endif
	}

//#define GEOMETRIC
	bool TriangleMesh::hitTriangle(uint i0, uint i1, uint i2, const ray& r, hitrecord& hr)
	{
		//both from http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/
		vec3 v0 = vertices[i0].pos;
		vec3 v1 = vertices[i1].pos;
		vec3 v2 = vertices[i2].pos;
#ifdef GEOMETRIC
		vec3 v0v1 = v1 - v0;
		vec3 v0v2 = v2 - v0;
		vec3 n = v0v1.cross(v0v2); //get normal from vertices instead?
		float ndr = n.dot(r.d);
		if (ndr == 0) return false;
		float d = n.dot(v0);
		float t = -(n.dot(r.e) + d) / ndr;
		vec3 ph = r.e + r.d*t;
		vec3 v0p = ph - v0;
		float v = n.dot(v0v1.cross(v0p));
		if(v < 0) return false;
		vec3 v1p = ph - v1;
		vec3 v1v2 = v2 - v1;
		float w = n.dot(v1v2.cross(v1p));
		if(w < 0) return false;
		vec3 v2p = ph - v2;
		vec3 v2v0 = v0 - v2;
		float u = n.dot(v2v0.cross(v2p));
		if (u < 0) return false;
		if (t < hr.t)
		{
			hr.t = t;
			float w = 1-(u+v);
			hr.norm = vertices[i0].norm * w + vertices[i1].norm * u + vertices[i2].norm*v;
			hr.textureCoord = vertices[i0].texc*w + vertices[i1].texc*u + vertices[i2].texc*v;
			hr.p = this;
			hr.pos = ph;
			return true;
		}
		return false;
#else

		float u, v;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 pv = r.d.cross(e2);
		float det = e1.dot(pv);
		if(det < -FLT_EPSILON || det < FLT_EPSILON)
		//if(det == 0)
			return false;
		float idet = 1.f / det;
		vec3 tv = r.e - v0;
		u = tv.dot(pv) * idet;
		if(u < 0 || u > 1.0f)
			return false;
		vec3 qv = tv.cross(e1);
		v = r.d.dot(qv) * idet;
		if(v < 0 || u+v > 1)
			return false;
		float nt = e2.dot(qv) * idet;
		if(nt > 0 && nt < hr.t)
		{
			hr.t = nt;
			float w = 1-(u+v);
			hr.norm = vertices[i0].norm * w + vertices[i1].norm * u + vertices[i2].norm*v;
			hr.textureCoord = vertices[i0].texc*w + vertices[i1].texc*v + vertices[i2].texc*u;
			hr.p = this;
			hr.pos = r.e + r.d*nt;
			return true;
		}
		return false;
#endif
	}

	bool TriangleMesh::hit(const ray& r, hitrecord& hr)
	{
		ray rd = ray(_inv_world.transform(r.e), norm(_inv_world.transform(r.d, 0)));
		if(_tree->hit(r, hr))
		{
			hr.norm = matrix_tranpose(_inv_world).transform(hr.norm, 0);
			return true;
		}
		return false;
	}

		
};