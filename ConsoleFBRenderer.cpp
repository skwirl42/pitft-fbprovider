#include "ConsoleFBRenderer.h"

#include <linux/fb.h>
#include <stdio.h>

#include "Console.h"

ConsoleFBRenderer::ConsoleFBRenderer(
		struct fb_var_screeninfo *fbinfo, uint16_t *framebuffer,
		bool *fontbuffer, uint16_t fontBufferWidth, uint16_t fontBufferHeight,
		uint16_t fontCharsWide, uint16_t fontCharsHigh,
		uint16_t foregroundColour, uint16_t backgroundColour, int cursorBlinkFrames)
	: 	info(fbinfo), framebuffer(framebuffer), fontbuffer(fontbuffer),
		fontBufferWidth(fontBufferWidth), fontBufferHeight(fontBufferHeight),
		fontCharsWide(fontCharsWide), fontCharsHigh(fontCharsHigh),
		foregroundColour(foregroundColour), backgroundColour(backgroundColour),
		cursorBlinkFrames(cursorBlinkFrames)
{
	charPixelsWide = fontBufferWidth / fontCharsWide;
	charPixelsHigh = fontBufferHeight / fontCharsHigh;
}

void ConsoleFBRenderer::Clear()
{
	auto framebufferSize = info->xres * info->yres;
	for (uint16_t i = 0; i < framebufferSize; i++)
	{
		framebuffer[i] = backgroundColour;
	}
}

void ConsoleFBRenderer::Render(Console *console, int frame)
{
	auto cursorOn = (frame / cursorBlinkFrames) % 2;

	int cursorX;
	int cursorY;
	console->GetCursor(cursorX, cursorY);

	console->Visit([&](int x, int y, char character, CharacterAttribute attribute)
	{
		auto xFBStart = x * charPixelsWide;
		auto yFBStart = y * charPixelsHigh;
		auto charXStart = (character % fontCharsWide) * charPixelsWide;
		auto charYStart = (character / fontCharsWide) * charPixelsHigh;
		auto isCursor = (x == cursorX) && (y == cursorY);

		for (uint16_t charLine = 0; charLine < charPixelsHigh; charLine++)
		{
			for (uint16_t charColumn = 0; charColumn < charPixelsWide; charColumn++)
			{
				auto charValue = fontbuffer[(charYStart + charLine) * fontBufferWidth + (charXStart + charColumn)];
				if (((int)attribute & (int)CharacterAttribute::Inverted) ^ (cursorOn && isCursor))
				{
					charValue = !charValue;
				}

				framebuffer[(yFBStart + charLine) * info->xres + xFBStart + charColumn] = charValue ? foregroundColour : backgroundColour;
			}
		}
	});
}

void ConsoleFBRenderer::SetColours(uint16_t foregroundColour, uint16_t backgroundColour)
{
	this->foregroundColour = foregroundColour;
	this->backgroundColour = backgroundColour;
}
