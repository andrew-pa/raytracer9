#pragma once
#include <cmath>
#include <math.h>
#include <stdlib.h>



	typedef unsigned int uint;
#define PI 3.14159265358979323846f

	inline float randf()
	{
		return ((float)rand() / (float)RAND_MAX);
	}

	inline float randfn()
	{
		return (randf()*2.f) - 1.f;
	}

	enum class Axis
	{
		X = 0, Y, Z
	};

#pragma region Vec3
	struct vec3
	{
	public:
		float x,  y, z;

		vec3()
			: x(0), y(0), z(0)
		{
		}
		
		explicit vec3(float xyz) 
			: x(xyz), y(xyz), z(xyz)
		{
		}

		vec3(float _x, float _y, float _z)
			: x(_x), y(_y), z(_z)
		{
		}

		float dot(vec3 b) const
		{
			return (x * b.x) + (y * b.y) + (z * b.z);
		}

		vec3 cross(vec3 b) const
		{
			return vec3((y * b.z) - (z * b.y),
							(z * b.x) - (x * b.z),
							(x * b.y) - (y * b.x));
		}

		inline float sqrlength() const
		{
			return dot(*this);
		}

		inline float length() const
		{
			return sqrtf(sqrlength());
		}

		inline void norm()
		{
			float rlen = 1/length();
			x *= rlen; y *= rlen; z *= rlen;
		}

		inline float& operator [](Axis i)
		{
			switch (i)
			{
			case Axis::X:
				return x;
			case Axis::Y:
				return y;
			case Axis::Z:
				return z;
			}
			throw i;
		}

		inline float& operator [](uint i)
		{
			switch (i)
			{
			case 0:
				return x;
			case 1:
				return y;
			case 2:
				return z;
			}
			throw i;
		}
	};

	static const vec3 vec3_up = vec3(0, 1, 0);
	static const vec3 vec3_right = vec3(1, 0, 0);
	static const vec3 vec3_back = vec3(0, 0, -1);

#pragma region Vec3 ops

	inline vec3 operator *(vec3 a, vec3 b)
	{
		return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
	}
	inline vec3 operator +(vec3 a, vec3 b)
	{
		return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
	}
	inline vec3 operator -(vec3 a, vec3 b)
	{
		return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
	}
	inline vec3 operator /(vec3 a, vec3 b)
	{
		return vec3(a.x / b.x, a.y / b.y, a.z / b.z);
	}
	inline vec3 operator -(vec3 a)
	{
		return vec3(-a.x, -a.y, -a.z);
	}
	inline vec3 operator *(vec3 a, float b)
	{
		return vec3(a.x * b, a.y * b, a.z * b);
	}
	inline vec3 operator *(float b, vec3 a)
	{
		return vec3(a.x * b, a.y * b, a.z * b);
	}
	inline vec3 operator /(vec3 a, float b)
	{
		float rb = 1.0f / b;
		return vec3(a.x * rb, a.y * rb, a.z * rb);
	}
	inline vec3 operator /(float b, vec3 a)
	{
		float rb = 1.0f / b;
		return vec3(a.x * rb, a.y * rb, a.z * rb);
	}
	inline bool operator >(vec3 a, vec3 b)
	{
		return (a.x > b.x) && (a.y > b.y) && (a.z > b.z);
	}
	inline bool operator <(vec3 a, vec3 b)
	{
		return (a.x < b.x) && (a.y < b.y) && (a.z < b.z);
	}
	inline bool operator ==(vec3 a, vec3 b)
	{
		return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
	}

	inline vec3 norm(vec3 a)
	{
		float rlen = 1.f / a.length();
		return a * rlen;
	}

	inline vec3 max(vec3 a, vec3 b)
	{
		return vec3(__max(a.x, b.x), __max(a.y, b.z), __max(a.z, b.z));
	}

	inline vec3 min(vec3 a, vec3 b)
	{
		return vec3(__min(a.x, b.x), __min(a.y, b.y), __min(a.z, b.z));
	}

	inline float max_comp(vec3 a)
	{
		return __max(a.x, __max(a.y, a.z));
	}

	inline vec3 reflect(vec3 d, vec3 n)
	{
		return d - 2.f  * (d.dot(n)) * n;
	}

	inline bool refract(vec3 d, vec3 n, float nt, vec3& t)
	{
		float idn = d.dot(n);
		float R = (1.f - (nt*nt)) * (1.f - (idn*idn));
		if(R <= 0)
		{
			return false;
		}
		else 
		{
			R = sqrtf(R);
			R = nt*idn + R;
			t = nt * (d-n) * R;
			return true;
		}
	}

	inline vec3 recip(vec3 v)
	{
		return vec3(1.f/v.x, 1.f/v.y, 1.f/v.z);
	}

#pragma endregion
#pragma endregion

#pragma region Vec2
	struct vec2
	{
	public:
		float x, y;

		vec2()
			: x(0), y(0) 
		{
		}

		vec2(float _x, float _y)
			: x(_x), y(_y)
		{
		}
	};

	inline vec2 operator +(vec2 a, vec2 b)
	{
		return vec2(a.x+b.x, a.y+b.y);
	}

	inline vec2 operator *(vec2 a, float b)
	{
		return vec2(a.x * b, a.y * b);
	}

	inline vec2 operator *(float b, vec2 a)
	{
		return vec2(a.x * b, a.y * b);
	}
#pragma endregion
	
	
	struct ray
	{
	public:
		vec3 e, d;
		
		ray(vec3 _e, vec3 _d)
			: e(_e), d(_d)
		{
		}
	};

	struct aabb
	{
	public:
		vec3 Min, Max;

		aabb()
			: Min(), Max() { }

		aabb(vec3 m, vec3 x)
			: Min(m), Max(x) { }
		aabb(aabb a, aabb b)
			: Min(min(a.Min, b.Min)),
			  Max(max(a.Max, b.Max))
		{ }
		aabb(vec3 center, float ext)
		{
			Min = center - vec3(ext);
			Max = center + vec3(ext);
		}

		inline vec3 Center() const
		{
			return (Min+Max) / 2.f;
		}
		inline float Extent() const
		{
			return max_comp(Max - Center());
		}

		inline float Area()
		{
			vec3 ex = Max - Min;
			return ex.x * ex.y * ex.z;
		}

		inline aabb& Enlarge(float s)
		{
			Min = Min*s;
			Max = Max*s;
			return *this;
		}

		inline void AddPoint(vec3 p)
		{
			if(p.x > Max.x) Max.x = p.x;
			if(p.y > Max.y) Max.y = p.y;
			if(p.z > Max.z) Max.z = p.z;

			if(p.x < Min.x) Min.x = p.x;
			if(p.y < Min.y) Min.y = p.y;
			if(p.z < Min.z) Min.z = p.z;
			
			//for(uint i = (uint)Axis::X; i < (uint)Axis::Z; ++i)
			//{
			//	if(p[(Axis)i] < Min[(Axis)i]) Min[(Axis)i] = p[(Axis)i];
			//	if(p[(Axis)i] > Max[(Axis)i]) Max[(Axis)i] = p[(Axis)i];
			//}
		}

		inline bool Inside(aabb b) const
		{
			if(Min.x >= b.Max.x) return false;
			if(Max.x <= b.Min.x) return false;
			if(Min.y >= b.Max.y) return false;
			if(Max.y <= b.Min.y) return false;
			if(Min.z >= b.Max.z) return false;
			if(Max.z <= b.Min.z) return false;
			return true;
		}

		inline bool Inside(vec3 p) const
		{
			if(p.x >= Min.x && p.x <= Max.x &&
				p.y >= Min.y && p.y <= Max.y &&
				p.z >= Min.z && p.z <= Max.z)
				return true;
			return false;
		}

		//intersection with a line segment
		inline bool intersects(vec3 s, vec3 end) const
		{
			if (Inside(s) || Inside(end))
				return true;
			vec3 d = (s - end) * .5f;
			vec3 e = (Max - Min) * .5f;
			vec3 c = s + d - (Min + Max) * .5f;
			vec3 ad;
			ad.x = fabsf(d.x); ad.y = fabsf(d.y); ad.z = fabsf(d.z);
			if (fabsf(c[0]) > e[0] + ad[0])
				return false;
			if (fabsf(c[1]) > e[1] + ad[1])
				return false;
			if (fabsf(c[2]) > e[2] + ad[2])
				return false;

			if (fabsf(d[1] * c[2] - d[2] * c[1]) > e[1] * ad[2] + e[2] * ad[1] + 0.0001f)
				return false;
			if (fabsf(d[2] * c[0] - d[0] * c[2]) > e[2] * ad[0] + e[0] * ad[2] + 0.0001f)
				return false;
			if (fabsf(d[0] * c[1] - d[1] * c[0]) > e[0] * ad[1] + e[1] * ad[0] + 0.0001f)
				return false;

			return true;
		}

#define FASTAABB
		inline bool Hit(ray r) const
		{
			if(r.e > Min && r.e < Max) return true;

#ifdef FASTAABB
			vec3 rrd = recip(r.d);
			float tx1 = (Min.x - r.e.x)*rrd.x;
			float tx2 = (Max.x - r.e.x)*rrd.x;

			float tmin = __min(tx1, tx2);
			float tmax = __max(tx1, tx2);

			float ty1 = (Min.y - r.e.y)*rrd.y;
			float ty2 = (Max.y - r.e.y)*rrd.y;

			tmin = __max(tmin, __min(ty1, ty2));
			tmax = __min(tmax, __max(ty1, ty2));

			float tz1 = (Min.z - r.e.z)*rrd.z;
			float tz2 = (Max.z - r.e.z)*rrd.z;

			tmin = __max(tmin, __min(tz1, tz2));
			tmax = __min(tmax, __max(tz1, tz2));

			return tmax >= tmin;
#else

			float txmin, txmax, tymin, tymax, tzmin, tzmax;
			vec3 rrd = recip(r.d);
			vec3 minorgdif = Min - r.e;
			vec3 maxorgdif = Max - r.e;
			if (rrd.x >= 0)
			{
				txmin = (minorgdif.x) * rrd.x;
				txmax = (maxorgdif.x) * rrd.x;
			}
			else
			{
				txmin = (maxorgdif.x) * rrd.x;
				txmax = (minorgdif.x) * rrd.x;
			}
			if (rrd.y >= 0)
			{
				tymax = (maxorgdif.y) * rrd.y;
				tymin = (minorgdif.y) * rrd.y;
			}
			else
			{
				tymax = (minorgdif.y) * rrd.y;
				tymin = (maxorgdif.y) * rrd.y;
			}
			if ((txmin > tymax) || (tymin > txmax))
				return false;
			if (tymin > txmin) txmin = tymin;
			if (tymax < txmax) txmax = tymax;
			
			if (rrd.z >= 0)
			{
				tzmax = (maxorgdif.z) * rrd.z;
				tzmin = (minorgdif.z) * rrd.z;
			}
			else
			{
				tzmax = (minorgdif.z) * rrd.z;
				tzmin = (maxorgdif.z) * rrd.z;
			}
			if ((txmin > tzmax) || (tzmin > tzmax))
				return false;
			if (tzmin > txmin)
				txmin = tzmin;
			if (tzmax < txmax)
				txmax = tzmax;
			return txmin > 0;
#endif
		}
	};

#pragma region matrix4x4
	//column order 4x4 matrix
	struct matrix4x4
	{
		//column, rows
		float m[4][4];
		matrix4x4() { }

		matrix4x4(float m00, float m10, float m20, float m30,
			 	  float m01, float m11, float m21, float m31,
				  float m02, float m12, float m22, float m32,
				  float m03, float m13, float m23, float m33)
		{
			m[0][0] = m00; m[1][0] = m10; m[2][0] = m20; m[3][0] = m30; 
			m[0][1] = m01; m[1][1] = m11; m[2][1] = m21; m[3][1] = m31; 
			m[0][2] = m02; m[1][2] = m12; m[2][2] = m22; m[3][2] = m32; 
			m[0][3] = m03; m[1][3] = m13; m[2][3] = m23; m[3][3] = m33; 
		}

		inline float* operator [](int c)
		{
			return (float*)m[c];
		}

		inline const float* operator [](int c) const
		{
			return (float*)m[c];
		}

		//  [x][...] = [x*m11 + y*m21 + z*m31 + w*m41]
		//  [y][...] = [x*m12 + y*m22 + z*m32 + w*m42]
		//  [z][...] = [x*m13 + y*m23 + z*m33 + w*m43]
		//  [w][...] = [x*m14 + y*m24 + z*m34 + w*m44]
		inline vec3 transform(vec3 a, float w = 1) const
		{
			vec3 ta;
			ta.x = a.x*m[0][0] + a.y*m[1][0] + a.z*m[2][0] + w*m[3][0];
			ta.y = a.x*m[0][1] + a.y*m[1][1] + a.z*m[2][1] + w*m[3][1];
			ta.z = a.x*m[0][2] + a.y*m[1][2] + a.z*m[2][2] + w*m[3][2];
			//ta.w = a.x*m[0][3] + a.y*m[1][3] + a.z*m[2][3] + w*m[3][3];
			return ta;
		}

		inline aabb transform(aabb box) const
		{
			vec3 min; vec3 max;
			min = vec3(m[3][0], m[3][1], m[3][2]);
			max = min;

			if(m[0][0] > 0) 
			{
				min.x += m[0][0]*box.Min.x; max.x += m[0][0]*box.Max.x; 
			}
			else
			{
				min.x += m[0][0]*box.Max.x; max.x += m[0][0]*box.Min.x; 
			}

			if(m[1][0] > 0) 
			{
				min.y += m[1][0]*box.Min.x; max.y += m[1][0]*box.Max.x; 
			}
			else
			{
				min.y += m[1][0]*box.Max.x; max.y += m[1][0]*box.Min.x; 
			}

			if(m[2][0] > 0) 
			{
				min.z += m[2][0]*box.Min.x; max.z += m[2][0]*box.Max.x; 
			}
			else
			{
				min.z += m[2][0]*box.Max.x; max.z += m[2][0]*box.Min.x; 
			}

			
			if(m[0][1] > 0) 
			{
				min.x += m[0][1]*box.Min.y; max.x += m[0][1]*box.Max.y; 
			}
			else
			{
				min.x += m[0][1]*box.Max.y; max.x += m[0][1]*box.Min.y; 
			}

			if(m[1][1] > 0) 
			{
				min.y += m[1][1]*box.Min.y; max.y += m[1][1]*box.Max.y; 
			}
			else
			{
				min.y += m[1][1]*box.Max.y; max.y += m[1][1]*box.Min.y; 
			}

			if(m[2][1] > 0) 
			{
				min.z += m[2][1]*box.Min.y; max.z += m[2][1]*box.Max.y; 
			}
			else
			{
				min.z += m[2][1]*box.Max.y; max.z += m[2][1]*box.Min.y; 
			}

			
			if(m[0][2] > 0) 
			{
				min.x += m[0][2]*box.Min.z; max.x += m[0][2]*box.Max.z; 
			}
			else
			{
				min.x += m[0][2]*box.Max.z; max.x += m[0][2]*box.Min.z; 
			}

			if(m[1][2] > 0) 
			{
				min.y += m[1][2]*box.Min.z; max.y += m[1][2]*box.Max.z; 
			}
			else
			{
				min.y += m[1][2]*box.Max.z; max.y += m[1][2]*box.Min.z; 
			}

			if(m[2][2] > 0) 
			{
				min.z += m[2][2]*box.Min.z; max.z += m[2][2]*box.Max.z; 
			}
			else
			{
				min.z += m[2][2]*box.Max.z; max.z += m[2][2]*box.Min.z; 
			}

			return aabb(min, max);
		}
	};



	inline matrix4x4 operator *(const matrix4x4& m, float b)
	{
		return matrix4x4(m[0][0]*b, m[1][0]*b,m[2][0]*b, m[3][0]*b, 
						 m[0][1]*b, m[1][1]*b,m[2][1]*b, m[3][1]*b, 
						 m[0][2]*b, m[1][2]*b,m[2][2]*b, m[3][2]*b,
						 m[0][3]*b, m[1][3]*b,m[2][3]*b, m[3][3]*b);
	}

	inline matrix4x4 operator *(float b, const matrix4x4& m)
	{
		return m*b;
	}

	inline matrix4x4 operator *(const matrix4x4& a, const matrix4x4& b)
	{
		matrix4x4 _c;
		for(int r = 0; r < 4; ++r)
			for(int c = 0; c < 4; ++c)
			{
				_c[c][r] = a[0][r]*b[c][0] + a[1][r]*b[c][1] + a[2][r]*b[c][2] + a[3][r]*b[c][3]; 
			}
		return _c;
	}

	inline matrix4x4 matrix_tranpose(const matrix4x4& m)
	{
		return matrix4x4(m[0][0], m[0][1], m[0][2], m[0][3], 
						 m[1][0], m[1][1], m[1][2], m[1][3], 
						 m[2][0], m[2][1], m[2][2], m[2][3],
						 m[3][0], m[3][1], m[3][2], m[3][3]);
	}

	inline matrix4x4 matrix_inverse(const matrix4x4& m)
	{
		float SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		float SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		float SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		float SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		float SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		float SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		float SubFactor06 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		float SubFactor07 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		float SubFactor08 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		float SubFactor09 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		float SubFactor10 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		float SubFactor11 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		float SubFactor12 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		float SubFactor13 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
		float SubFactor14 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
		float SubFactor15 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
		float SubFactor16 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
		float SubFactor17 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
		float SubFactor18 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		matrix4x4 Inverse(			
			+ m[1][1] * SubFactor00 - m[1][2] * SubFactor01 + m[1][3] * SubFactor02,
			- m[1][0] * SubFactor00 + m[1][2] * SubFactor03 - m[1][3] * SubFactor04,
			+ m[1][0] * SubFactor01 - m[1][1] * SubFactor03 + m[1][3] * SubFactor05,
			- m[1][0] * SubFactor02 + m[1][1] * SubFactor04 - m[1][2] * SubFactor05,

			- m[0][1] * SubFactor00 + m[0][2] * SubFactor01 - m[0][3] * SubFactor02,
			+ m[0][0] * SubFactor00 - m[0][2] * SubFactor03 + m[0][3] * SubFactor04,
			- m[0][0] * SubFactor01 + m[0][1] * SubFactor03 - m[0][3] * SubFactor05,
			+ m[0][0] * SubFactor02 - m[0][1] * SubFactor04 + m[0][2] * SubFactor05,

			+ m[0][1] * SubFactor06 - m[0][2] * SubFactor07 + m[0][3] * SubFactor08,
			- m[0][0] * SubFactor06 + m[0][2] * SubFactor09 - m[0][3] * SubFactor10,
			+ m[0][0] * SubFactor11 - m[0][1] * SubFactor09 + m[0][3] * SubFactor12,
			- m[0][0] * SubFactor08 + m[0][1] * SubFactor10 - m[0][2] * SubFactor12,

			- m[0][1] * SubFactor13 + m[0][2] * SubFactor14 - m[0][3] * SubFactor15,
			+ m[0][0] * SubFactor13 - m[0][2] * SubFactor16 + m[0][3] * SubFactor17,
			- m[0][0] * SubFactor14 + m[0][1] * SubFactor16 - m[0][3] * SubFactor18,
			+ m[0][0] * SubFactor15 - m[0][1] * SubFactor17 + m[0][2] * SubFactor18);

		float det = 
			+ m[0][0] * Inverse[0][0] 
			+ m[0][1] * Inverse[1][0] 
			+ m[0][2] * Inverse[2][0] 
			+ m[0][3] * Inverse[3][0];

		float idet = 1.0f / det;
		Inverse = Inverse*idet;
		return Inverse;
	}

	inline matrix4x4 matrix_idenity()
	{
		return matrix4x4(1, 0, 0, 0, 
						 0, 1, 0, 0, 
						 0, 0, 1, 0,
						 0, 0, 0, 1);
	}

	inline matrix4x4 matrix_translate(vec3 t)
	{
		return matrix4x4(1, 0, 0, t.x,
						 0, 1, 0, t.y,
						 0, 0, 1, t.z,
						 0, 0, 0, 1);
	}

	inline matrix4x4 matrix_rotation_x(float x)
	{
		return matrix4x4(1, 0,		 0,        0, 
						 0, cosf(x), -sinf(x), 0, 
						 0, sinf(x),  cosf(x), 0,
						 0, 0,		  0,	   1);
	}

	inline matrix4x4 matrix_rotation_y(float y)
	{
		return matrix4x4(cosf(y), 0, sinf(y),0, 
						 0,		  1, 0,		 0, 
						 -sinf(y),0, cosf(y),0,
						 0,		  0, 0,	     1);
	}

	inline matrix4x4 matrix_rotation_z(float z)
	{
		return matrix4x4(cosf(z), -sinf(z), 0, 0, 
						 sinf(z),  cosf(z), 0, 0, 
						 0,		   0,       1, 0,
						 0,        0,       0, 1);
	}
#pragma endregion


