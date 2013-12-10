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

namespace windows
{
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef DrawText
};

#include <PixelToaster.h>
class DisplayTexture2D : public Texture2D, public PixelToaster::Listener
{
	PixelToaster::Display* display;
	vector<PixelToaster::Pixel> pxs;
	mutex pxlk;
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
		pxlk.lock();
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
		pxlk.unlock();
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

	inline void lock() {
		return pxlk.lock();
	}
	inline void unlock() {
		return pxlk.unlock();
	}
};

vec3 cosdistrib(vec3 n)
{
	vec3 w = n;
	vec3 u, v;
	make_orthonormal_frame(w, u, v);
	float e1 = randf();
	float e2 = randf();
	float se2 = sqrtf(e2);
	float t2e = 2 * PI* e1;
	return norm((cosf(t2e)*se2*u) +
		(sinf(t2e)*se2*v) +
		(sqrtf(1 - e2)*w));
}


class Renderer
{
public:
	BVHNode* scene;
	Camera cam;
	DisplayTexture2D* renderTarget;

	Renderer(BVHNode* s, Camera c, DisplayTexture2D* rt)
		: scene(s), cam(c), renderTarget(rt) { }

	const float sample_count = 16.f;
	//const float bounce_count = 4.f;
	const int   depth_count  = 4;

	uint ray_count = 0;

	//double max_pix_time = 0.f;

	vec3 raycolor(const ray& r, hitrecord& hr, int depth = 0)
	{
		if (depth > depth_count) return vec3(0, 0, 0);
		ray_count++;
		if (scene->hit(r, hr))
		{
			//return vec3(1);
			if (hr.p->mat()->emit.sqrlength() > 0)
				return hr.p->mat()->emit;
			//	vec3 rd = cosdistrib(hr.norm);
			hitrecord hrr;
			hrr.t = 1000000;
			vec3 c = vec3(0);// (hr.p->mat()->diffuse * raycolor(ray(r.e + ((hr.t - 0.01f)*r.d), rd), hrr, depth + 1));
			vec3 dt = hr.p->mat()->diffuse * (hr.p->mat()->tex != nullptr ? hr.p->mat()->tex->Texel(hr.textureCoord) : vec3(1));
		//	for (float i = 0; i < bounce_count; ++i)
		//	{
			vec3 v =  (dt * raycolor(ray(r.e + ((hr.t - 0.01f)*r.d), cosdistrib(hr.norm)), hrr, depth + 1));
				return v;
				//c = c + v;
		//	}
		//	c = c * (1.f / bounce_count);
			return c;
		}
		else
		{
			vec3 v = vec3(0.2f, 0.2f, 0.2f);// +(r.d.dot(vec3(0, 1, 0))*vec3(0, 0.3f, 1));
			//v.x = 0;
			return v;
		}
	}

	inline void render_chunk(vec2 start, vec2 size)
	{
		ray r(vec3(0), vec3(0));
		hitrecord hr;
		//renderTarget->Pixel(0, y) = vec3(.3f + (y / renderTarget->Height()));
		for (float y = start.y; y < size.y+start.y && y < renderTarget->Height(); ++y)
		{
			for (float x = start.x; x < size.x+start.x && x < renderTarget->Width(); ++x)
			{
				//auto st = (double)clock();
				for (float p = 0; p < sample_count; ++p)
				{
					for (float q = 0; q < sample_count; ++q)
					{
						r = cam.generateRay(x + ((q+randfn()) * (1.f / sample_count)), y + ((p+randfn()) * (1.f / sample_count)));
						hr.t = 1000000;
						renderTarget->Pixel(x, y) = renderTarget->Pixel(x, y) + raycolor(r, hr);
					}
				}
				renderTarget->Pixel(x, y) = renderTarget->Pixel(x, y) * (1.f / (sample_count*sample_count));
				//auto et = (double)clock();
				//st = et - st;
				//max_pix_time = __max(st, max_pix_time);
				//vec3 c;
			//	if (st <= 20)
				///	c = vec3(0, st / 20.0, 0);
				//else if (st > 20 && st <= 40)
				//	c = vec3(0, 0, (st-15.0) / 20.0);
				//else if (st > 40 && st <= 60)
				//	c = vec3((st-30.0) / 20.0, 0, 0);
			}
		}
	}

#define STD_THREAD
	void render(vec2 tile_size = vec2(16, 16))
	{
#ifdef STD_THREAD
		mutex tileque_mutex;
		queue<vec2> tiles;

		vector<vec2> prtiles;
		for (float y = 0; y < renderTarget->Height(); y += tile_size.y)
		{
			for (float x = 0; x < renderTarget->Width(); x += tile_size.x)
			{
				prtiles.push_back(vec2(x, y));
			}
		}
		for (int i = 0; i < 20 + prtiles.size(); ++i)
		{
			swap(prtiles[rand() % prtiles.size()], prtiles[rand() % prtiles.size()]);
		}

		for (auto& t : prtiles)
			tiles.push(t);

		vector<thread> threads;
		for (int i = 0; i < thread::hardware_concurrency(); ++i)
		{
			threads.push_back(thread([&]
			{
				uint chunks_rendered = 0;
				while (!tiles.empty())
				{
					unique_lock<mutex> lm(tileque_mutex);
					vec2 t = tiles.front(); tiles.pop();
					lm.unlock();
					render_chunk(t, tile_size);
					chunks_rendered++;
					cout << "T" << this_thread::get_id() << " % " << tiles.size() << endl;
				}
				cout << "Thread " << this_thread::get_id() << " rendered " << chunks_rendered << " chunks" << endl;
			}));
		}
		
		for (auto& t : threads)
		{
			t.join();
			this_thread::sleep_for(std::chrono::milliseconds(100));
		}
#else 
		vector<vec2> tiles;
		for (float y = 0; y < renderTarget->Height(); y += 32)
		{
			for (float x = 0; x < renderTarget->Width(); x += 32)
			{
				tiles.push_back(vec2(x, y));
			}
		}
		concurrency::parallel_for_each(tiles.begin(), tiles.end(), [&](vec2 t) 
		{
			render_chunk(t);
			cout << "Rendered chunk (" << t.x << ", " << t.y << ")" << endl;
		});
#endif
	}
};



//int retdmain(int argc, char* argv[])
//{
//	srand(time(nullptr));
//	DisplayTexture2D tex(320, 240);
//	
//	Camera cam(vec3(0, 5, -10), vec3(0), tex.Width(), tex.Height());
//
//	//CheckerTexture ctx(vec3(0, 1, 0), vec3(1, 1, 0));
//	Texture2D ctx("tex.bmp");
//
//	vector<Primitive*> prims;
//	//for(int i = 0; i < 16; ++i)
//	//	prims.push_back(new Sphere(vec3(randfn()*6, randfn()*6, randfn()*6), 0.5f));
//	auto tmbt = (double)clock();
//	for(int i = 0; i < 16; ++i)
//		prims.push_back(new TriangleMesh("tri.obj", new material(vec3(randfn(), randfn(), randfn()), vec3((rand() % 8 != 0 ? 0 : 1))),
//			matrix_rotation_x(randfn()*2*PI) * 
//			matrix_rotation_y(randfn()*2*PI) *
//			matrix_rotation_z(randfn()*2*PI) * 
//			matrix_translate(vec3(randfn() * 8, randfn() * 8, randfn()*8))));// , 8, 48));
//	auto ltmbt = (double)clock();
//	tmbt = ltmbt - tmbt;
//	cout << "Tree build for trimesh took " << tmbt << " clocks" << endl;
//	//OctreeNode ot = OctreeNode(aabb(vec3(-10), vec3(10)), prims, 16);
//	scenetree = new BVHNode(prims);
//
//	double avg_render_time = 0.0f;
//	uint render_count = 0;
//
//	float camTheta = 0.f;
//	float camPhi = 0.f;
//
//	float t = 0.f;
//	time_t st;
//	float x, y;
//	ray r(vec3(0), vec3(0));
//	hitrecord hr;
//	
//	while(tex.Display()->open())
//	{
//		st = (double)clock();
//
//		for(y = 0; y < tex.Height(); ++y)
//		{
//			for(x = 0; x < tex.Width(); ++x)
//			{
//				r = cam.generateRay(x + randfn()*.1f, y + randfn()*.1f);
//				hr.t = 1000000;
//				tex.Pixel(x, y) = (tex.Pixel(x,y) + raycolor(r, hr)) * .5f;
//				/*if(ot.hit(r, hr))
//				{
//					float d = __max(0, hr.norm.dot(vec3(.4f, .6f, 0))) + 0.1f;
//					tex.Pixel(x, y) = d*ctx.Texel(hr.textureCoord);
//				}
//				else
//				{
//					tex.Pixel(x, y) = vec3(0, .0f, .3f) + (r.d.dot(vec3(0, 1, 0))*vec3(0, 0.4f, 1));
//				}*/
//		}
//		//tex.Update();
//	}
//
//	auto et = (double)clock();
//
//	cout << "Render took " << (et - st) << " clocks (" << (et - st)/(double)CLOCKS_PER_SEC << " seconds)" << endl;
//	//cout << tex.mouseCoords().x << " " << tex.mouseCoords().y << endl;
//	avg_render_time += (et - st);
//	render_count++;
//	tex.Update();
//	t += (et - st)/(float)CLOCKS_PER_SEC;//0.016f;
//
//	//if(tex.deltaMouseCoords().x != 0)
//	//	camTheta -= tex.deltaMouseCoords().x*4;
//	//if(tex.deltaMouseCoords().y != 0)
//	//	camPhi -= tex.deltaMouseCoords().y*4;
//
//	//cam.lookAt(vec3(
//	//	20 * sinf(camPhi) * cosf(camTheta), 
//	//	20 * cosf(camPhi), 
//	//	20 * sinf(camPhi) * sinf(camTheta)  ), vec3(0));
//	//ctx.Color1.z = sin(t*.5f);
//	//ctx.Color2.y = 1-sin(t*.5f);
//	}
//
//	avg_render_time /= (double)render_count;
//	cout << "Avg render time " << avg_render_time << endl;
//
//	ostringstream fnm;
//	fnm << "img" << time(nullptr) << ".bmp";
//	tex.SaveToBMP(fnm.str().c_str());
//
//	getchar();
//	return 0;
//}

int main(int argc, char* argv[])
{
	srand(time(nullptr));
	DisplayTexture2D tex(160, 120);
	Camera cam(vec3(0, 5, -7), vec3(0), tex.Width(), tex.Height());

	CheckerTexture ctx(vec3(0, .5f, 0), vec3(.5f, .5f, 0));
	//Texture2D ctx("tex.bmp");

	vector<Primitive*> prims;
	//for(int i = 0; i < 16; ++i)
	//	prims.push_back(new Sphere(vec3(randfn()*6, randfn()*6, randfn()*6), 0.5f));
	//auto tmbt = (double)clock();
	//	for (int i = 0; i < 16; ++i)
	//		prims.push_back(new TriangleMesh("tri.obj", new material(vec3(randfn(), randfn(), randfn()), vec3((rand() % 8 != 0 ? 0 : 1))),
	//		matrix_rotation_x(randfn() * 2 * PI) *
	//		matrix_rotation_y(randfn() * 2 * PI) *
	//		matrix_rotation_z(randfn() * 2 * PI) *
	//		matrix_translate(vec3(randfn() * 8, randfn() * 8, randfn() * 8))));// , 8, 48));
	//	prims.push_back(new Sphere(vec3(0, 3, 0), .5f, new material(vec3(0), vec3(10.f))));
	//prims.push_back(new TriangleMesh("open_box.obj", new material(vec3(0.7f, 0.7f, 0.7f), vec3(0.0f),
	//	nullptr)));
	prims.push_back(new TriangleMesh("taj.obj", new material(vec3(0.7f, 0.7f, 0.3f), vec3(0.0f), 
		&ctx )
		, matrix_translate(vec3(0, -2, 0)) ));
//	prims.push_back(new Sphere(vec3(-1, -2, 0), 0.5f, new material(vec3(0, 1, 0), vec3(0))));
	//prims.push_back(new Sphere(vec3(0, -2, 0), 0.5f, new material(vec3(.7f, .7f, .7f), vec3(0))));
//	prims.push_back(new Sphere(vec3(1, -2, 0), 0.5f, new material(vec3(1, 0, 0), vec3(0))));

	prims.push_back(new TriangleMesh("light.obj", new material(vec3(0, 0, 0), vec3(6.0f)),
		/*matrix_scale(vec3(1, .1f, 1)) **/ matrix_translate(vec3(0, 2.0f, 0))));
	//auto ltmbt = (double)clock();
//	tmbt = ltmbt - tmbt;
//	cout << "Tree build for trimesh took " << tmbt << " clocks" << endl;
	//OctreeNode ot = OctreeNode(aabb(vec3(-10), vec3(10)), prims, 16);
	Renderer renderer = Renderer(new BVHNode(prims), cam, &tex);

	//float t = 0.f;
	//time_t st;
	//float x//, y;
//	ray r(vec3(0), vec3(0));
//	hitrecord hr;
//	auto st = (double)clock();
	windows::LARGE_INTEGER frq;
	windows::QueryPerformanceFrequency(&frq);
	cout << "Render starting" << endl;
	windows::LARGE_INTEGER st;
	windows::QueryPerformanceCounter(&st);

	renderer.render(vec2(32, 32));
	windows::LARGE_INTEGER et;
	windows::QueryPerformanceCounter(&et);
	auto cycles = (et.QuadPart - st.QuadPart);
	double secs = (double)cycles / (double)frq.QuadPart;
	//const float sample_count = 4.f;
	//concurrency::parallel_for(0, (int)tex.Height(), [&](int _y) 
	//{
	//	ray r(vec3(0), vec3(0));
	//	hitrecord hr;
	//	float y = (float)_y;
	//	tex.Pixel(0, y) = vec3(.3f + (y / tex.Height()));
	//	for (float x = 1; x < tex.Width(); ++x)
	//	{
	//		for (float p = 0; p < sample_count; ++p)
	//		{
	//			for (float q = 0; q < sample_count; ++q)
	//			{
	//				r = cam.generateRay(x + (p*randfn()) / sample_count, y + (p*randfn()) / sample_count);
	//				hr.t = 1000000;
	//				tex.Pixel(x, y) = tex.Pixel(x, y) + renderer.raycolor(r, hr);
	//			}
	//		}
	//		tex.Pixel(x, y) = tex.Pixel(x, y) * 1.f / (sample_count*sample_count);
	//	}
	//	//cout << "Scanline " << _y << endl;
	//});

	//for (y = 0; y < tex.Height(); ++y)
	//{
	//	tex.Pixel(0, y) = vec3(.3f + (y / tex.Height()));
	//	for (x = 1; x < tex.Width(); ++x)
	//	{
	//		for (float p = 0; p < sample_count; ++p)
	//		{
	//			for (float q = 0; q < sample_count; ++q)
	//			{
	//				r = cam.generateRay(x + (p*randfn()) / sample_count, y + (p*randfn()) / sample_count);
	//				hr.t = 1000000;
	//				tex.Pixel(x, y) = tex.Pixel(x, y) + raycolor(r, hr);
	//			}
	//		}
	//		tex.Pixel(x, y) = tex.Pixel(x, y) * 1.f / (sample_count*sample_count);
	//	}
	//	if((int)y % 2 == 0)
	//		tex.Update();
	//}


//	auto et = (double)clock();

	cout << "Render took " << cycles << " clocks (" << secs << " seconds)" << endl;
	cout << "Shot " << renderer.ray_count << " rays" << endl;
	double rc = renderer.ray_count;
	double avg_ray_time = cycles / rc;
	cout << "Avg ray took " << avg_ray_time << " clocks" << endl;
	//cout << "Max pixel time " << renderer.max_pix_time << " clocks" << endl;

	ostringstream txt;
	txt << "RENDER TOOK " << cycles << " CLOCKS :: " << secs << " SECONDS" << endl;
	txt << "SHOT " << renderer.ray_count << " RAYS" << endl;
	txt << "AVG RAY TOOK " << avg_ray_time << " CLOCKS" << endl;
	DrawText(&tex, txt.str(), vec2(0, 0), vec3(.8f, .8f, .8f));

	ostringstream fnm;
	fnm << "img" << time(nullptr) << ".bmp";
	tex.SaveToBMP(fnm.str().c_str());
	while (tex.Display()->open()) tex.Update();

	return 0;
}