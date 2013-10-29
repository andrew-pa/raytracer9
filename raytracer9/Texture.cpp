#include "Texture.h"
#include "bitmap.h"

#pragma region STB_IMAGE_WRITE
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

static void writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
	  switch (*fmt++) {
		 case ' ': break;
		 case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
		 case '2': { int x = va_arg(v,int); unsigned char b[2];
					 b[0] = (unsigned char) x; b[1] = (unsigned char) (x>>8);
					 fwrite(b,2,1,f); break; }
		 case '4': { stbiw_uint32 x = va_arg(v,int); unsigned char b[4];
					 b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
					 b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
					 fwrite(b,4,1,f); break; }
		 default:
			assert(0);
			return;
	  }
   }
}

static void write3(FILE *f, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a, arr[1] = b, arr[2] = c;
   fwrite(arr, 3, 1, f);
}

static void write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   stbiw_uint32 zero = 0;
   int i,j,k, j_end;

   if (y <= 0)
	  return;

   if (vdir < 0) 
	  j_end = -1, j = y-1;
   else
	  j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
	  for (i=0; i < x; ++i) {
		 unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
		 if (write_alpha < 0)
			fwrite(&d[comp-1], 1, 1, f);
		 switch (comp) {
			case 1:
			case 2: write3(f, d[0],d[0],d[0]);
					break;
			case 4:
			   if (!write_alpha) {
				  // composite against pink background
				  for (k=0; k < 3; ++k)
					 px[k] = bg[k] + ((d[k] - bg[k]) * d[3])/255;
				  write3(f, px[1-rgb_dir],px[1],px[1+rgb_dir]);
				  break;
			   }
			   /* FALLTHROUGH */
			case 3:
			   write3(f, d[1-rgb_dir],d[1],d[1+rgb_dir]);
			   break;
		 }
		 if (write_alpha > 0)
			fwrite(&d[comp-1], 1, 1, f);
	  }
	  fwrite(&zero,scanline_pad,1,f);
   }
}

static int outfile(char const *filename, int rgb_dir, int vdir, int x, int y, int comp, void *data, int alpha, int pad, const char *fmt, ...)
{
   FILE *f;
   if (y < 0 || x < 0) return 0;
   fopen_s(&f, filename, "wb");
   //f = fopen(filename, "wb");
   if (f) {
	  va_list v;
	  va_start(v, fmt);
	  writefv(f, fmt, v);
	  va_end(v);
	  write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad);
	  fclose(f);
   }
   return f != NULL;
}

int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   int pad = (-x*3) & 3;
   return outfile(filename,-1,-1,x,y,comp,(void *) data,0,pad,
		   "11 4 22 4" "4 44 22 444444",
		   'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
			40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

#pragma endregion

namespace raytracer9
{
	vec3 conv(const RGBA& rgba)
	{
		return vec3((float)rgba.Red/255.f, (float)rgba.Green/255.f, (float)rgba.Blue/255.f);
	}

	Texture2D::Texture2D(const char* bmpFilename)
	{
		CBitmap bitmap = CBitmap(bmpFilename);
		w = bitmap.GetWidth();
		h = bitmap.GetHeight();
		RGBA* imgdata = (RGBA*)bitmap.GetBits();
		pixels = new vec3[w*h];
		for(int i = 0; i < w*h; ++i)
		{
			pixels[i] = conv(imgdata[i]);
		}
		bitmap.Dispose();
	}

	Texture2D::Texture2D(int _w, int _h, ITexture* from)
		: w(_w), h(_h)
	{
		pixels = new vec3[w*h];
		for(int y = 0; y < h; ++y)
		{
			for(int x = 0; x < w; ++x)
			{
				pixels[(y*w)+x] = from->Pixel(x, y);
			}
		}
	}

	void Texture2D::SaveToBMP(const char* bmpFilename)
	{
		//CBitmap bitmap;
		unsigned char* imgdata = new unsigned char[w*h*4];
		for(int y = 0; y < h*4; y+=4)
		{
			for(int x = 0; x < w*4; x+=4)
			{
				vec3 d = min(max(pixels[(y*w + x)/4], vec3()), vec3(1));
				imgdata[(y*w + x)] = (unsigned char)(d.x * 255.f);
				imgdata[(y*w + x)+1] = (unsigned char)(d.y * 255.f);
				imgdata[(y*w + x)+2] = (unsigned char)(d.z * 255.f);
				imgdata[(y*w + x)+3] = 255;
			}
		}
		//bitmap.SetBits(imgdata, w, h, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		//bitmap.Save(bmpFilename);
		//bitmap.Dispose();
		stbi_write_bmp(bmpFilename, w, h, 4, imgdata);
	}
}