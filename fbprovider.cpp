/*
 * fbprovider.c
 * 
 * Copyright 2020  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdint.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <png++/png.hpp>

void printbitfield(const char *name, struct fb_bitfield *bitfield)
{
	printf("%s bitfield: %d %d %d\n", name, bitfield->offset, bitfield->length, bitfield->msb_right);
}

int main(int argc, char **argv)
{
	int fb = open("/dev/fb1", O_RDWR);
	struct fb_var_screeninfo info;
	if (fb > 0 && ioctl(fb, FBIOGET_VSCREENINFO, &info) == 0)
	{
		printf("xres: %d, yres: %d, bpp: %d, grayscale: 0x%x\n", info.xres, info.yres, info.bits_per_pixel, info.grayscale);
		printbitfield("red", &info.red);
		printbitfield("green", &info.green);
		printbitfield("blue", &info.blue);
		printbitfield("transparent", &info.transp);
		printf("dimensions: %d x %d\n", info.width, info.height);
		
		size_t bufferSize = info.bits_per_pixel * info.xres * info.yres;
		uint16_t *buffer = mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
		uint16_t value = 0xF0F0;
		for (int y = 0; y < info.yres; y++)
		{
			for (int x = 0; x < info.xres; x++)
			{
				if (x % 0x1F > 16)
				{
					value = 0xF0F0;
				}
				else
				{
					value = 0x0000;
				}
				
				if (y % 0x1F > 16)
				{
					value ^= 0xFFFF;
				}
				//value = (((x >> 2) % 1) ^ ((y >> 2) % 1)) == 0 ? 0xFFFF : 0;
				//printf("%4x", value);
				buffer[y * info.xres + x] = value;
				//buffer[y * info.xres + x] = 0;
			}
		}
		munmap(buffer, bufferSize);
	}
	else
	{
		printf("Failed to open the framebuffer!\n");
	}
	return 0;
}

