#pragma once
#include "Math.h"
#include "Texture.h"

namespace raytracer9
{

class Primitive;

struct hitrecord
{
	Primitive* p;
	vec3 pos;
	float t;
	vec3 norm;
	vec2 textureCoord;

	hitrecord() { }

	hitrecord(Primitive* _p, vec3 pi, float xt, vec3 n, vec2 tc)
		: p(_p), pos(pi), t(xt), norm(n), textureCoord(tc) { }
};

class Primitive
{
public:
	virtual bool hit(const ray& r, hitrecord& hr) = 0;
	virtual aabb bounds() const = 0;
	virtual vec3 center() = 0;
	virtual bool containedBy(const aabb& b) = 0;
	virtual bool intersects(const aabb& b)
	{
		return bounds().Inside(b);
	}
};

class Sphere : public Primitive
{
	vec3 _c;
	float _r;

	vec2 getUV(vec3 pi, vec3 n)
	{
		float u,v;
		float phi = acosf(-(n.dot(vec3_up)));
		v = phi * (1.f / PI);
		float theta = (acosf(vec3_back.dot(n) / sinf(phi))) * (2.f/PI);
		if(vec3_right.dot(n) >= 0) u = (1-theta);
		else u = theta;
		return vec2(u,v);
	}
public:
	Sphere(vec3 c, float r)
		: _c(c), _r(r) { }

	bool hit(const ray& r, hitrecord& hr) override
	{
		vec3 v = r.e - _c;
		float b = - v.dot(r.d);
		float det = (b*b) - v.dot(v) + (_r*_r);
		if(det > 0)
		{
			det = sqrtf(det);
			float i1 = b - det;
			float i2 = b + det;
			if(i2 > 0)
			{
				if(i1 < 0)
				{
					if(i2 < hr.t)
					{
						hr.t = i2;
						return false;
					}
				}
				else
				{
					if(i1 < hr.t)
					{
						hr.t = i1;
						vec3 pi = r.e + r.d*hr.t;
						vec3 n = norm(pi-_c);
						vec2 uv = getUV(pi, n);
						hr.p = this;
						hr.pos = pi;
						hr.norm = n;
						hr.textureCoord = uv;
						return true;
					}
				}
			}
		}
		return false;
	}

	inline aabb bounds() const override
	{
		auto a = aabb(_c - vec3(_r), _c + vec3(_r));	
		return a;
	}

	inline vec3 center() override { return _c; }
	inline vec3& center_() { return _c; }
	inline float& radius() { return _r; }
	inline bool containedBy(const aabb& b) override { return bounds().Inside(b); }
};

};