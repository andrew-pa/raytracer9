#include "stdinc.h"

#pragma once
#include "Math.h"
#include "Texture.h"
#include "Camera.h"
using namespace raytracer9;

#include "Primitive.h"
#include "Octree.h"
#include "BVHTree.h"
#include "TriangleMesh.h"

#include <PixelToaster.h>
class DisplayTexture2D : public Texture2D, public PixelToaster::Listener
{
	PixelToaster::Display* display;
	vector<PixelToaster::Pixel> pxs;
	vec2 mc;
	vec2 dmc;
public:
	DisplayTexture2D(float w, float h)
		: Texture2D(w, h),
		display(new PixelToaster::Display("raytracer9", w, h)),
		mc(0,0)
	{
		display->listener(this);
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
		dmc = vec2();
		display->update(pxs);
	}

	inline PixelToaster::Display* Display() { return display; }

	inline vec2 mouseCoords() {	return mc; }
	inline vec2 deltaMouseCoords() { return dmc; }

	void onMouseMove(PixelToaster::DisplayInterface&, PixelToaster::Mouse m) override
	{
		static vec2 lmc;
		mc.x = m.x/display->width();
		mc.y = m.y/display->height();
		dmc = mc - lmc;
		lmc = mc;
	}
};

int main(int argc, char* argv[])
{
	srand(time(nullptr));
	DisplayTexture2D tex(320, 240);
	
	Camera cam(vec3(0, 5, -10), vec3(0), tex.Width(), tex.Height());

	//CheckerTexture ctx(vec3(0, 1, 0), vec3(1, 1, 0));
	Texture2D ctx("tex.bmp");

	vector<Primitive*> prims;
	//for(int i = 0; i < 16; ++i)
	//	prims.push_back(new Sphere(vec3(randfn()*6, randfn()*6, randfn()*6), 0.5f));
	auto tmbt = (double)clock();
	for(int i = 0; i < 16; ++i)
		prims.push_back(new TriangleMesh("ship.obj", 
			matrix_rotation_x(randfn()*2*PI) * 
			matrix_rotation_y(randfn()*2*PI) *
			matrix_rotation_z(randfn()*2*PI) * 
			matrix_translate(vec3(randfn() * 16, randfn() * 16, randfn()*16))));// , 8, 48));
	auto ltmbt = (double)clock();
	tmbt = ltmbt - tmbt;
	cout << "Tree build for trimesh took " << tmbt << " clocks" << endl;
	//OctreeNode ot = OctreeNode(aabb(vec3(-10), vec3(10)), prims, 16);
	BVHNode ot = BVHNode(prims);

	double avg_render_time = 0.0f;
	uint render_count = 0;

	float camTheta = 0.f;
	float camPhi = 0.f;

	float t = 0.f;
	time_t st;
	float x, y;
	ray r(vec3(0), vec3(0));
	hitrecord hr;
	
	while(tex.Display()->open())
	{
		st = (double)clock();

		for(y = 0; y < tex.Height(); ++y)
		{
			for(x = 0; x < tex.Width(); ++x)
			{
				r = cam.generateRay(x, y);
				hr.t = 1000000;
				if(ot.hit(r, hr))
				{
					float d = __max(0, hr.norm.dot(vec3(.4f, .6f, 0))) + 0.1f;
					tex.Pixel(x, y) = d*ctx.Texel(hr.textureCoord);
				}
				else
				{
					tex.Pixel(x, y) = vec3(0, .0f, .3f) + (r.d.dot(vec3(0, 1, 0))*vec3(0, 0.4f, 1));
				}
		}
		//tex.Update();
	}

	auto et = (double)clock();

	cout << "Render took " << (et - st) << " clocks (" << (et - st)/(double)CLOCKS_PER_SEC << " seconds)" << endl;
	//cout << tex.mouseCoords().x << " " << tex.mouseCoords().y << endl;
	avg_render_time += (et - st);
	render_count++;
	tex.Update();
	t += (et - st)/(float)CLOCKS_PER_SEC;//0.016f;

	if(tex.deltaMouseCoords().x != 0)
		camTheta -= tex.deltaMouseCoords().x*4;
	if(tex.deltaMouseCoords().y != 0)
		camPhi -= tex.deltaMouseCoords().y*4;

	cam.lookAt(vec3(
		20 * sinf(camPhi) * cosf(camTheta), 
		20 * cosf(camPhi), 
		20 * sinf(camPhi) * sinf(camTheta)  ), vec3(0));
	//ctx.Color1.z = sin(t*.5f);
	//ctx.Color2.y = 1-sin(t*.5f);
	}

	avg_render_time /= (double)render_count;
	cout << "Avg render time " << avg_render_time << endl;

	ostringstream fnm;
	fnm << "img" << time(nullptr) << ".bmp";
	tex.SaveToBMP(fnm.str().c_str());

	getchar();
	return 0;
}