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

	void DrawText(Texture2D* texture, const std::string& text, vec2 pos, vec3 color)
	{
		const int char_width = 5;
		const int char_height = 7;
		static std::map<char, string>* chars = nullptr;
		if (chars == nullptr)
		{
			//bulid chars map
			chars = new std::map<char, string>;
#pragma region ABCs
			(*chars)['A'] =
				"xxxxx"
				"x...x"
				"x...x"
				"xxxxx"
				"x...x"
				"x...x"
				"x...x";
			(*chars)['B'] =
				"xxxx."
				"x...x"
				"x...x"
				"xxxx."
				"x...x"
				"x...x"
				"xxxx.";
			(*chars)['C'] =
				".xxxx"
				"x...."
				"x...."
				"x...."
				"x...."
				"x...."
				".xxxx";
			(*chars)['D'] =
				"xxxx."
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"xxxx.";
			(*chars)['E'] =
				"xxxxx"
				"x...."
				"x...."
				"xxxxx"
				"x...."
				"x...."
				"xxxxx";
			(*chars)['F'] =
				"xxxxx"
				"x...."
				"x...."
				"xxxxx"
				"x...."
				"x...."
				"x....";
			(*chars)['G'] =
				"xxxxx"
				"x...."
				"x...."
				"x..xx"
				"x...x"
				"x...x"
				"xxxxx";
			(*chars)['H'] =
				"x...x"
				"x...x"
				"x...x"
				"xxxxx"
				"x...x"
				"x...x"
				"x...x";
			(*chars)['I'] =
				"xxxxx"
				"..x.."
				"..x.."
				"..x.."
				"..x.."
				"..x.."
				"xxxxx";
			(*chars)['J'] =
				"xxxxx"
				"...x."
				"...x."
				"...x."
				"...x."
				"x..x."
				".xx..";
			(*chars)['K'] =
				"x...x"
				"x..x."
				"x.x.."
				"xx..."
				"x.x.."
				"x..x."
				"x...x";
			(*chars)['L'] =
				"x...."
				"x...."
				"x...."
				"x...."
				"x...."
				"x...."
				"xxxxx";
			(*chars)['M'] =
				"xxxxx"
				"x.x.x"
				"x.x.x"
				"x.x.x"
				"x.x.x"
				"x...x"
				"x...x";
			(*chars)['N'] =
				"x...x"
				"xx..x"
				"xx..x"
				"x.x.x"
				"x..xx"
				"x..xx"
				"x...x";
			(*chars)['O'] =
				"xxxxx"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"xxxxx";
			(*chars)['P'] =
				"xxxxx"
				"x...x"
				"x...x"
				"xxxxx"
				"x...."
				"x...."
				"x....";
			(*chars)['Q'] =
				"xxxxx"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"xxxx."
				"....x";
			(*chars)['R'] =
				"xxxxx"
				"x...x"
				"x...x"
				"xxxxx"
				"x.x.."
				"x..x."
				"x...x";
			(*chars)['S'] =
				"xxxxx"
				"x...."
				"x...."
				"xxxxx"
				"....x"
				"....x"
				"xxxxx";
			(*chars)['T'] =
				"xxxxx"
				"..x.."
				"..x.."
				"..x.."
				"..x.."
				"..x.."
				"..x..";
			(*chars)['U'] =
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"xxxxx";
			(*chars)['V'] =
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				"x...x"
				".xxx.";
			(*chars)['W'] =
				"x...x"
				"x...x"
				"x...x"
				"x.x.x"
				"x.x.x"
				"x.x.x"
				"xxxxx";
			(*chars)['X'] =
				"x...x"
				".x.x."
				".x.x."
				"..x.."
				".x.x."
				".x.x."
				"x...x";
			(*chars)['Y'] =
				"x...x"
				"x...x"
				"x...x"
				"xxxxx"
				"..x.."
				"..x.."
				"..x..";
			(*chars)['Z'] =
				"xxxxx"
				"....x"
				"...x."
				"...x."
				"..x.."
				".x..."
				"xxxxx";

			(*chars)['0'] =
				"xxxxx"
				"xx..x"
				"x.x.x"
				"x.x.x"
				"x.x.x"
				"x..xx"
				"xxxxx";
			(*chars)['1'] =
				".xx.."
				"x.x.."
				"..x.."
				"..x.."
				"..x.."
				"..x.."
				"xxxxx";
			(*chars)['2'] =
				".xxx."
				"x...x"
				"....x"
				"...x."
				".xx.."
				"x...."
				"xxxxx";
			(*chars)['3'] =
				"xxxx."
				"....x"
				"....x"
				"..xx."
				"....x"
				"....x"
				"xxxx.";
			(*chars)['4'] =
				"...xx"
				".x..x"
				"x...x"
				"xxxxx"
				"....x"
				"....x"
				"....x";
			(*chars)['5'] =
				"xxxxx"
				"x...."
				"x...."
				"xxxx."
				"....x"
				"....x"
				"xxxx.";
			(*chars)['6'] =
				".xxxx"
				"x...."
				"x...."
				"xxxx."
				"x...x"
				"x...x"
				".xxx."; 
			(*chars)['7'] =
				"xxxxx"
				"....x"
				"...x."
				"...x."
				"..x.."
				".x..."
				"x...."; 
			(*chars)['8'] =
				".xxx."
				"x...x"
				"x...x"
				".xxx."
				"x...x"
				"x...x"
				".xxx."; 
			(*chars)['9'] =
				".xxx."
				"x...x"
				"x...x"
				".xxxx"
				"....x"
				"....x"
				"xxxx.";
			(*chars)[':'] =
				"..x.."
				"..x.."
				"..x.."
				"....."
				"..x.."
				"..x.."
				"..x..";
			(*chars)['.'] =
				"....."
				"....."
				"....."
				"....."
				"....."
				"xx..."
				"xx...";
#pragma endregion
		}
		vec2 texpos = pos;
		for (auto c : text)
		{
			if (c == ' ')
			{
				texpos.x += char_width + 2;
				continue;
			}
			if (c == '\n')
			{
				texpos.x = pos.x;
				texpos.y += char_height + 1;
				continue;
			}
			string chrm = (*chars)[c];
			vec2 cpos = vec2(0, 0);
			for (auto c : chrm)
			{
				if(c == 'x')
					texture->Pixel(cpos.x + texpos.x, cpos.y + texpos.y) = color;
				cpos.x++;
				if (cpos.x >= char_width)
				{
					cpos.x = 0;
					cpos.y++;
					if (cpos.y > char_height)
						break;
				}
			}
			texpos.x += char_width + 2;
		}

	}
}