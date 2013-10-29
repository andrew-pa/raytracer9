#pragma once
#include "Math.h"

namespace raytracer9
{
	class ITexture
	{
	public:
		inline virtual vec3& Pixel(int x, int y) = 0;
		inline virtual vec3& Texel(vec2 uv) = 0;
		inline virtual float Height() const = 0;
		inline virtual float Width() const = 0;
	};

	class Texture2D : public ITexture
	{
	protected:
		vec3* pixels;
		int h, w;
	public:
		Texture2D(int _w, int _h)
			: w(_w), h(_h) 
		{
			pixels = new vec3[w*h];
		}

		Texture2D(const char* bmpFilename);

		Texture2D(int _w, int _h, ITexture* from);

		void SaveToBMP(const char* filename);

		inline vec3& Pixel(int x, int y) override
		{
			return pixels[(y*w) + x];
		}

		inline vec3& Texel(vec2 uv) override
		{
			if(uv.x > 1) uv.x = .95f;
			else if(uv.x < 0) uv.x = 0;
			if(uv.y > 1) uv.y = .95f;
			else if(uv.y < 0) uv.y = 0;
			int x = (int)floorf(uv.x*(float)w);
			int y = (int)floorf(uv.y*(float)h);
			return Pixel(x,y);
		}

		inline float Width() const override { return (float)w; }
		inline float Height() const override { return (float)h; }
	};

	class CheckerTexture : public ITexture
	{
		float h, w;
	public:
		inline float Width() const override { return w; }
		inline float Height() const override { return h; }

		vec3 Color1, Color2;
		float CheckerSize;

		CheckerTexture(vec3 col1, vec3 col2, float checkerSize = 8.f, float _w = 64.f, float _h = 64.f)
			: Color1(col1), Color2(col2), w(_w), h(_h), CheckerSize(checkerSize)
		{
		}

		inline vec3& Pixel(int x, int y) override
		{
			return Texel(vec2((float)x / w, (float)y / h));
		}

		inline vec3& Texel(vec2 uv) override
		{
			float ch = floorf(uv.x*CheckerSize) + floorf(uv.y*CheckerSize);
			ch = (float)((int)ch % 2);
			if(ch == 0)
				return Color1;
			return Color2;
		}
	};

	//class XYLinGradTexture : public ITexture
	//{
	//	float h, w;
	//public:
	//	inline float Width() const override { return w; }
	//	inline float Height() const override { return h; }

	//	vec3 Color1, Color2;
	//	float CheckerSize;

	//	XYLinGradTexture(vec3 col1, vec3 col2, float checkerSize = 8.f, float _w = 64.f, float _h = 64.f)
	//		: Color1(col1), Color2(col2), w(_w), h(_h), CheckerSize(checkerSize)
	//	{
	//	}

	//	inline vec3& Pixel(int x, int y) override
	//	{
	//		return Texel(vec2((float)x / w, (float)y / h));
	//	}

	//	inline vec3& Texel(vec2 uv) override
	//	{
	//		return vec3(1, 0, 0);
	//	}
	//};
}