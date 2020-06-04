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
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <png++/png.hpp>

#include "Console.h"
#include "ConsoleFBRenderer.h"

const int fontCharsPerLine = 16;
const int fontCharsLines = 8;

/*
	red bitfield: 11 5 0
	green bitfield: 5 6 0
	blue bitfield: 0 5 0
 */
void printbitfield(const char *name, struct fb_bitfield *bitfield)
{
	printf("%s bitfield: %d %d %d\n", name, bitfield->offset, bitfield->length, bitfield->msb_right);
}

uint16_t value_to_bitfield(png::byte value, struct fb_bitfield *bitfield)
{
	return value >> (8 - bitfield->length) << bitfield->offset;
}

uint16_t pixel_to_bitfields(png::rgb_pixel pixel, struct fb_var_screeninfo *info)
{
	return value_to_bitfield(pixel.red, &info->red)
		| value_to_bitfield(pixel.green, &info->green)
		| value_to_bitfield(pixel.blue, &info->blue);
}

int main(int argc, char **argv)
{
	png::image<png::gray_pixel> font("/home/pi/repos/robco-os/font/robco-termfont.png");
	auto charWidth = font.get_width() / fontCharsPerLine;
	auto charHeight = font.get_height() / fontCharsLines;
	printf("font dimensions: %d x %d\n", font.get_width(), font.get_height());
	printf("char height %d, char width %d\n", charHeight, charWidth);
	auto fontBuffer = new bool[font.get_width() * font.get_height()];
	for (uint16_t y = 0; y < font.get_height(); y++)
	{
		for (uint16_t x = 0; x < font.get_width(); x++)
		{
			fontBuffer[y * font.get_width() + x] = font.get_pixel(x, y) > 0;
		}
	}

	int fb = open("/dev/fb1", O_RDWR);
	struct fb_var_screeninfo info;
	if (fb > 0 && ioctl(fb, FBIOGET_VSCREENINFO, &info) == 0)
	{
		auto charsWide = info.xres / charWidth;
		auto charsTall = info.yres / charHeight;
		printf("Console dimensions: %d x %d\n", charsWide, charsTall);

		printf("\n");
		printf("xres: %d, yres: %d, bpp: %d, grayscale: 0x%x\n", info.xres, info.yres, info.bits_per_pixel, info.grayscale);
		printbitfield("red", &info.red);
		printbitfield("green", &info.green);
		printbitfield("blue", &info.blue);
		printbitfield("transparent", &info.transp);
		printf("fb dimensions: %d x %d\n", info.width, info.height);

		size_t bufferSize = info.bits_per_pixel * info.xres * info.yres;
		uint16_t *buffer = reinterpret_cast<uint16_t*>(mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0));
		auto colourBg = 0x0000;
		auto colourFg = 0x07E0;

		Console console(charsWide, charsTall);
		console.SetCursor(0, 0);
		console.SetCurrentAttribute(CharacterAttribute::Inverted);
		console.PrintLine("Testing the console functionality");
		console.SetCurrentAttribute(CharacterAttribute::None);
		console.PrintLine("THE QUICK BROWN FOX JUMPED OVER THE LAZY DOG");
		console.PrintLine("the quick brown fox jumped over the lazy dog");
		console.PrintLine("01234567890123456789");
		console.PrintLine("!@#$%^&*()_+-=[]{}\\/|,.<>?;:\"'");
		console.PrintLine("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed");
		console.PrintLine("do eiusmod tempor incididunt ut labore et dolore magna");
		console.PrintLine("aliqua. Ut enim ad minim veniam, quis nostrud exercitation");
		console.PrintLine("ullamco laboris nisi ut aliquip ex ea commodo consequat.");
		console.PrintLine("Duis aute irure dolor in reprehenderit in voluptate velit");
		console.PrintLine("esse cillum dolore eu fugiat nulla pariatur. Excepteur sint");
		console.PrintLine("occaecat cupidatat non proident, sunt in culpa qui officia");
		console.PrintLine("deserunt mollit anim id est laborum.");

		ConsoleFBRenderer renderer(&info, buffer, fontBuffer, font.get_width(), font.get_height(), fontCharsPerLine, fontCharsLines, colourFg, colourBg, 1);

		for (int i = 0; i < 120; i++)
		{
			renderer.Render(&console, i);
			usleep(500);
		}

		munmap(buffer, bufferSize);
	}
	else
	{
		printf("Failed to open the framebuffer!\n");
	}
	return 0;
}

