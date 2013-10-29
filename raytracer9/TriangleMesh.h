#pragma once
#include "stdinc.h"
#include "Math.h"
#include "Primitive.h"
#include "Octree.h"

namespace raytracer9
{
	class TriangleMesh : public Primitive
	{
	public:
		struct vertex
		{
		public:
			vec3 pos, norm;
			vec2 texc;
			vertex() { }
			vertex(vec3 p, vec3 n, vec2 t)
				: pos(p), norm(n), texc(t) { }
		};
	protected:
		OctreeNode* _tree;
		matrix4x4 _world;
		matrix4x4 _inv_world;
		
		vector<vertex> vertices;
		vector<uint> indices;
	public:
		class Triangle : public Primitive
		{
		public:
			TriangleMesh* mesh;
			//indices into vertex array, not the index array!
			uint i0, i1, i2;
			aabb _b;

			Triangle(uint _0, uint _1, uint _2, TriangleMesh* m)
				: i0(_0), i1(_1), i2(_2), mesh(m) 
			{
				_b = aabb(min(mesh->vertices[i0].pos, min(mesh->vertices[i1].pos, mesh->vertices[i2].pos)),
					max(mesh->vertices[i0].pos, max(mesh->vertices[i1].pos, mesh->vertices[i2].pos)));
			}

			inline aabb bounds() const override
			{
				return _b;
			}

			inline vec3 center() override
			{
				const float _1over3 = 1.f / 3.f;
				return _1over3 * (mesh->vertices[i0].pos + 
					mesh->vertices[i1].pos + mesh->vertices[i1].pos);
			}

			inline bool hit(const ray& r, hitrecord& hr) override
			{
				return mesh->hitTriangle(i0, i1, i2, r, hr);
			}

			inline bool inside(const aabb& b)
			{
				return _b.Inside(b);
			//	return b.Inside(center());
			//	return b.Inside(mesh->vertices[i0].pos) || 
			//		b.Inside(mesh->vertices[i1].pos) || 
			//		b.Inside(mesh->vertices[i2].pos);
			}

		};

		TriangleMesh(const vector<vertex>& v, const vector<uint>& i, const matrix4x4& w = matrix_idenity(),
			int octree_depth = 16, uint octree_primTarget = 4);
		TriangleMesh(const string& obj_file, const matrix4x4& w = matrix_idenity(),
			int octree_depth = 16, uint octree_primTarget = 4);
		

		bool hitTriangle(uint i0, uint i1, uint i2, const ray& r, hitrecord& hr);

		bool hit(const ray& r, hitrecord& hr) override;

		inline OctreeNode*& tree() { return _tree; }
		inline const matrix4x4& world() { return _world; }
		inline void world(const matrix4x4& w) { _world = w; _inv_world = matrix_inverse(w); }

		inline aabb bounds() const override
		{
			return _tree->bounds();
		}
		inline vec3 center() override
		{
			return _tree->center();
		}
		inline bool inside(const aabb& b) override
		{
			return _tree->bounds().Inside(b);
		}
	};
};
