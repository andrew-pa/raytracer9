#include "stdinc.h"

#pragma once
#include "Math.h"
#include "Texture.h"
#include "Camera.h"
using namespace raytracer9;

#include "Primitive.h"
#include "Octree.h"
#include "TriangleMesh.h"

#include <PixelToaster.h>
class DisplayTexture2D : public Texture2D
{
	PixelToaster::Display* display;
	vector<PixelToaster::Pixel> pxs;
public:
	DisplayTexture2D(float w, float h)
		: Texture2D(w, h),
		display(new PixelToaster::Display("raytracer9", w, h))
	{

	}

	inline void Update()
	{
		pxs = vector<PixelToaster::Pixel>(w*h);
		for(int y = 0; y < h; ++y)
		{
			for(int x = 0; x < w; ++x)
			{
				(pxs)[y*w+x].r = pixels[y*w+x].x;
				(pxs)[y*w+x].g = pixels[y*w+x].y;
				(pxs)[y*w+x].b = pixels[y*w+x].z;
			}
		}
		display->update(pxs);
	}

	inline PixelToaster::Display* Display() { return display; }
};

int main(int argc, char* argv[])
{
	DisplayTexture2D tex(320, 240);
	Camera cam(vec3(0, 5, -10), vec3(0), tex.Width(), tex.Height());

	CheckerTexture ctx(vec3(0, 1, 0), vec3(1, 1, 0));

	vector<Primitive*> prims;
	for(int i = 0; i < 8; ++i)
		prims.push_back(new Sphere(vec3(randfn()*6, randfn()*6, randfn()*6), 0.5f));
	auto tmbt = (double)clock();
	prims.push_back(new TriangleMesh("bunny.obj", matrix_idenity(), 8, 48));
	auto ltmbt = (double)clock();
	tmbt = ltmbt - tmbt;
	cout << "Tree build for trimesh took " << tmbt << " clocks" << endl;
	OctreeNode ot = OctreeNode(aabb(vec3(-10), vec3(10)), prims, 16);

	Sphere s(vec3(0,0,0), 1);

	double avg_render_time = 0.0f;
	uint render_count = 0;

	float t = 0.f;
	while(tex.Display()->open())
	{
	auto st = (double)clock();

	for(float y = 0; y < tex.Height(); ++y)
	{
		for(float x = 0; x < tex.Width(); ++x)
		{
			ray r = cam.generateRay(x, y);
			hitrecord hr;
			hr.t = 1000000;
			if(ot.hit(r, hr))
			{
				float d = __max(0, hr.norm.dot(vec3(.4f, .6f, 0)));
				tex.Pixel(x, y) = d*ctx.Texel(hr.textureCoord);
			}
			else
			{
				tex.Pixel(x, y) = vec3(0, .3f, .3f) + (r.d.dot(vec3(0, 1, 0))*vec3(0, 0.4f, 1));
			}
		}
		//tex.Update();
	}

	auto et = (double)clock();

	cout << "Render took " << (et - st) << " clocks (" << (et - st)/(double)CLOCKS_PER_SEC << " seconds)" << endl;
	avg_render_time += (et - st);
	render_count++;
	tex.Update();
	t += (et - st)/(float)CLOCKS_PER_SEC;//0.016f;
	cam.lookAt(vec3(10*sin(t), 5*sin(t), 10*cos(t)), vec3(0));
	ctx.Color1.z = sin(t*.5f);
	ctx.Color2.y = 1-sin(t*.5f);
	}

	avg_render_time /= (double)render_count;
	cout << "Avg render time " << avg_render_time << endl;

	ostringstream fnm;
	fnm << "img" << time(nullptr) << ".bmp";
	tex.SaveToBMP(fnm.str().c_str());

	//getchar();
	return 0;
}