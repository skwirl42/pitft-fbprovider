#pragma once

#include <stdint.h>

struct fb_var_screeninfo;
class Console;

class ConsoleFBRenderer
{
public:
	ConsoleFBRenderer(
		struct fb_var_screeninfo *fbinfo, uint16_t *framebuffer,
		bool *fontbuffer, uint16_t fontBufferWidth, uint16_t fontBufferHeight,
		uint16_t fontCharsWide, uint16_t fontCharsHigh,
		uint16_t foregroundColour, uint16_t backgroundColour,
		int cursorBlinkFrames);

	void Clear();
	void Render(Console *console, int frame);
	void SetColours(uint16_t foregroundColour, uint16_t backgroundColour);

private:
	struct fb_var_screeninfo *info;
	uint16_t *framebuffer;
	bool *fontbuffer;
	uint16_t fontBufferWidth;
	uint16_t fontBufferHeight;
	uint16_t fontCharsWide;
	uint16_t fontCharsHigh;
	uint16_t charPixelsWide;
	uint16_t charPixelsHigh;
	uint16_t foregroundColour;
	uint16_t backgroundColour;
	int cursorBlinkFrames;
};
