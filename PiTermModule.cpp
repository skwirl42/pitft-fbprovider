#include "PiTermModule.h"

#include <cstring>
#include <stdint.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include <png++/png.hpp>

#include "Console.h"
#include "ConsoleFBRenderer.h"

extern "C" {

typedef struct
{
	int file;
	struct fb_var_screeninfo fbinfo;
	uint16_t *framebuffer;
	size_t framebufferSize;
	bool *fontbuffer;
	Console *console;
	ConsoleFBRenderer *renderer;
	uint16_t fontbufferWidth;
	uint16_t fontbufferHeight;
	uint16_t fontCharsWide;
	uint16_t fontCharsHigh;
} PiTermData;

static PiTermData ModuleData;

const int fontCharsPerLine = 16;
const int fontCharsLines = 8;

static PyObject *PiTermError;

static void dispose()
{
	if (ModuleData.console != nullptr)
	{
		delete ModuleData.console;
	}
	if (ModuleData.renderer != nullptr)
	{
		delete ModuleData.renderer;
	}
	if (ModuleData.fontbuffer != nullptr)
	{
		delete [] ModuleData.fontbuffer;
	}
	if (ModuleData.framebuffer != nullptr)
	{
		munmap(ModuleData.framebuffer, ModuleData.framebufferSize);
	}
	if (ModuleData.file > 0)
	{
		close(ModuleData.file);
	}
	std::memset(&ModuleData, 0, sizeof(ModuleData));
}

static PyObject *PiTerm_alloc(PyObject *self, PyObject *args)
{
	bool success = false;
	const char *fontName;
	if (PyArg_ParseTuple(args, "s", &fontName))
	{
		png::image<png::gray_pixel> font(fontName);
		ModuleData.fontbuffer = new bool[font.get_width() * font.get_height()];
		for (uint16_t y = 0; y < font.get_height(); y++)
		{
			for (uint16_t x = 0; x < font.get_width(); x++)
			{
				ModuleData.fontbuffer[y * font.get_width() + x] = font.get_pixel(x, y) > 0;
			}
		}
		ModuleData.fontbufferWidth = font.get_width();
		ModuleData.fontbufferHeight = font.get_height();
		ModuleData.fontCharsWide = fontCharsPerLine;
		ModuleData.fontCharsHigh = fontCharsLines;
	}

	ModuleData.file = open("/dev/fb1", O_RDWR);
	if (ModuleData.file > 0 && ioctl(ModuleData.file, FBIOGET_VSCREENINFO, &ModuleData.fbinfo) == 0)
	{
		ModuleData.framebufferSize = ModuleData.fbinfo.bits_per_pixel * ModuleData.fbinfo.xres * ModuleData.fbinfo.yres;
		ModuleData.framebuffer = reinterpret_cast<uint16_t*>(mmap(nullptr, ModuleData.framebufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, ModuleData.file, 0));
		if (ModuleData.framebuffer == nullptr)
		{
			PyErr_SetString(PiTermError, "Failed to map the framebuffer");
		}
		else
		{
			ModuleData.console = new Console(60, 24);
			ModuleData.renderer = new ConsoleFBRenderer(&ModuleData.fbinfo, ModuleData.framebuffer,
														ModuleData.fontbuffer, ModuleData.fontbufferWidth, ModuleData.fontbufferHeight,
														ModuleData.fontCharsWide, ModuleData.fontCharsHigh,
														// Green foreground, black background, no cursor blink
														0x07E0, 0x0000, 0);
			if (ModuleData.console != nullptr && ModuleData.renderer != nullptr)
			{
				success = true;
			}
			else
			{
				PyErr_SetString(PiTermError, "Failed to create console resources");
			}
		}
	}
	else
	{
		PyErr_SetString(PiTermError, "Failed to open the framebuffer and/or get its info");
	}

	if (success)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}
	else
	{
		dispose();
		return nullptr;
	}
}

static PyObject *PiTerm_dispose(PyObject *self, PyObject *args)
{
	dispose();

	Py_INCREF(Py_None);
	return Py_None;
}

//~ def set_character(self, x, y, ch):
static PyObject *PiTerm_set_character(PyObject *self, PyObject *args)
{
	int x;
	int y;
	char character;
	if (PyArg_ParseTuple(args, "iib", &x, &y, &character))
	{
		ModuleData.console->SetChar(x, y, character);
		Py_INCREF(Py_None);
		return Py_None;
	}

	return nullptr;
}

//~ def print_str(self, x, y, string):
static PyObject *PiTerm_print_str(PyObject *self, PyObject *args)
{
	int x;
	int y;
	const char *string;
	if (PyArg_ParseTuple(args, "iis", &x, &y, &string))
	{
		ModuleData.console->PrintAt(string, x, y);
		Py_INCREF(Py_None);
		return Py_None;
	}

	return nullptr;
}

//~ def invert_at(self, x, y):
static PyObject *PiTerm_invert_at(PyObject *self, PyObject *args)
{
	int x;
	int y;
	if (PyArg_ParseTuple(args, "ii", &x, &y))
	{
		ModuleData.console->SetAttribute(CharacterAttribute::Inverted, x, y);
		Py_INCREF(Py_None);
		return Py_None;
	}

	return nullptr;
}

//~ def clear(self):
static PyObject *PiTerm_clear(PyObject *self, PyObject *args)
{
	ModuleData.console->Clear();
	Py_INCREF(Py_None);
	return Py_None;
}

//~ def refresh(self):
static PyObject *PiTerm_refresh(PyObject *self, PyObject *args)
{
	// Not incrementing the framecounter since we won't be blinking the cursor yet
	ModuleData.renderer->Render(ModuleData.console, 0);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef PiTermMethods[] = {
    {"alloc", PiTerm_alloc, METH_VARARGS, "Allocate needed resources. Takes a string containing a path to the terminal font (expects png with 16x8 characters, of 8x13 pixels each)"},
    {"dispose", PiTerm_dispose, METH_VARARGS, "Dispose of any allocated resources"},
    {"refresh", PiTerm_refresh, METH_VARARGS, "Flushes the contents of the console to the screen"},
    {"clear", PiTerm_clear, METH_VARARGS, "Clears the console, but does not render it to the screen"},
    {"invert_at", PiTerm_invert_at, METH_VARARGS, "Sets the inverted attribute on the character at x,y"},
    {"print_str", PiTerm_print_str, METH_VARARGS, "Prints a string starting at x,y"},
    {"set_character", PiTerm_set_character, METH_VARARGS, "Sets the character at x,y"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef PiTermModule = {
    PyModuleDef_HEAD_INIT,
    "PiTerm",   /* name of module */
    nullptr,	/* module documentation, may be NULL */
    -1,			/* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    PiTermMethods
};

PyObject* PyInit_PiTerm()
{
	PyObject *module;
	module = PyModule_Create(&PiTermModule);

	if (module == nullptr)
	{
		return nullptr;
	}

	PiTermError = PyErr_NewException("PiTerm.error", nullptr, nullptr);
	Py_XINCREF(PiTermError);
	if (PyModule_AddObject(module, "error", PiTermError) < 0)
	{
        Py_XDECREF(PiTermError);
        Py_CLEAR(PiTermError);
        Py_DECREF(module);
        return nullptr;
    }

	return module;
}

} // extern "C"
