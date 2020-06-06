#pragma once

#include <stdint.h>

struct fb_var_screeninfo;
class Console;

class ConsoleFBRenderer
{
public:
	///
	// Creates a renderer that will render characters from the font defined in `fontbuffer`
	// to `framebuffer`, using the Console object passed into Render(), and using colours
	// defined in `foregroundColour` and `backgroundColour`.
	// The framebuffer is assumed to be RGB565
	// `cursorBlinkFrames` defines how many frames will be rendered before the cursor toggles
	// blink state. Render's parameter `frame` is used to determine
	///
	ConsoleFBRenderer(
		struct fb_var_screeninfo *fbinfo, uint16_t *framebuffer,
		bool *fontbuffer, uint16_t fontBufferWidth, uint16_t fontBufferHeight,
		uint16_t fontCharsWide, uint16_t fontCharsHigh,
		uint16_t foregroundColour, uint16_t backgroundColour,
		int cursorBlinkFrames);

	void Clear();
	void SetColours(uint16_t foregroundColour, uint16_t backgroundColour);

	void Render(Console *console, int frame);

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
