#include "TriangleMesh.h"
#include <hash_map>
#include <fstream>
#include <iomanip>

namespace raytracer9
{
	TriangleMesh::TriangleMesh(const vector<vertex>& v,
		const vector<uint>& i, const matrix4x4& w, int octree_depth, uint octree_primTarget)
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
		_tree = new OctreeNode(bnds, ts, octree_depth, octree_primTarget);
	}

	struct ovt
	{
		int ip, it, in;
		ovt(int a = 0, int b = 0, int c = 0)
			: ip(a), it(c), in(b)
		{
		}

		bool operator ==(ovt b)
		{
			return (ip==b.ip) && (it==b.it) && (in==b.in);
		}
		bool operator !=(ovt b)
		{
			return ip!=b.ip && it!=b.it && in!=b.in;
		}
	};
	bool operator <(const ovt& a, const ovt& b)
	{
		return (a.ip < b.ip) || (a.in < b.in) || (a.it < b.it);
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

	TriangleMesh::TriangleMesh(const string& obj_file, const matrix4x4& w,
		int octree_depth, uint octree_primTarget)
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
		for(uint ix = 0; ix+3 < indices.size(); ix+=3)
		{
			ts.push_back(new Triangle(indices[ix], indices[ix+1], indices[ix+2], this));
		}
		_tree = new OctreeNode(bnds, ts, octree_depth, octree_primTarget);
	}

	
	bool TriangleMesh::hitTriangle(uint i0, uint i1, uint i2, const ray& r, hitrecord& hr)
	{
		uint z0 = i0;
		uint z1 = i1;
		uint z2 = i2;

		vec3 v0 = vertices[z0].pos;
		vec3 v1 = vertices[z1].pos;
		vec3 v2 = vertices[z2].pos;

		float u, v;
		vec3 e1 = v1 - v0;
		vec3 e2 = v2 - v0;
		vec3 pv = r.d.cross(e2);
		float det = e1.dot(pv);
		if(det == 0)
			return false;
		float idet = 1.f / det;
		vec3 tv = r.e - v0;
		u = tv.dot(pv) * idet;
		if(u < 0 || u > 1)
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
			hr.norm = vertices[z0].norm * w + vertices[z1].norm * u + vertices[z2].norm*v;
			hr.textureCoord = vertices[z0].texc*w + vertices[z1].texc*u + vertices[z2].texc*v;
			hr.p = this;
			hr.pos = r.e + r.d*nt;
			return true;
		}
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