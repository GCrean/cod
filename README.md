Cod

Cod is a simple library for rendering graphics and processing input on
Windows, Linux, and OSX. It supports loading PNGs and bitmap fonts
created by [Bitmap Font Generator](http://www.angelcode.com/products/bmfont/). It can be
compiled down to a single file for easy inclusion into your
projects. It is released under the [Boost Software License](http://www.boost.org/LICENSE_1_0.txt)

[Cod on Github](http://github.com/ioddly/cod)

![Screenshot](http://ioddly.com/static/cod/screenshot.png)

_Behold the awesome graphical powers of Cod!_

## Documentation

### Compiling

To include Cod in your projects, I recommend taking advantage of the
amalgamation feature. Simply use the following command in the cod
directory:

```shell
make cod-all.h
```

And a file named 'cod-all.h' will be generated. Then copy this file
into your project. To build with cod-all.h, you must define COD_LINK
in one (and only one) source file that includes cod-all.h. I recommend
creating a separate cod.c file with the following contents:

```c
#define COD_LINK
#include "cod-all.h"
```

When compiling this file, you must also define COD_PLATFORM to the
name of the platform you are building for. COD_PLATFORM can be one of
COD_X11, COD_WINDOWS, or COD_COCOA. Finally, you need to link the
appropriate libraries. On a Windows platform, building the program for
Windows is sufficient. On an X11 platform, link libX11 (you can use
'`pkg-config --libs x11`' to link appropriate libraries in your build
scripts). On Cocoa, use '`-framework Cocoa`', and compile the file as
Objective-C.

### Functions

As a rule of thumb, functions return 0 on failure. *cod_get_error*
will return a string describing the most recent error. 

---

```c
int cod_open(int width, int height)
```

Initialize Cod and open a window with the given dimensions.

---

```c
void cod_close()
```

Close the window and shut down Cod.

---

```c
void cod_set_title(const char*)
```

Change the title of the window.

---

```c
const char* cod_get_error(void)
```

Return a message describing the most recent error.

---

```c
int cod_get_event(cod_event* event)
```

If there's an event waiting to be processed, place it in
event. Returns 0 if there are no events.

---

```c
void cod_swap()
```

Draw to the window.

---

```c
void cod_clear()
```

Black out the window.

---

#### Images

```c
cod_image* cod_load_image(const char* path)
```

Load a PNG from the given path.

---

```c
void cod_free_image(cod_image*)
```

Free the image.

---

#### Fonts

```c
cod_font* cod_load_font(const char* fnt_path, const char* png_path)
```

Load a Bitmap Font Generator font; requires both the path to the fnt
file and the png file.

---

```c
void cod_free_font(cod_font* font
```

Free the font.

---

```c
void cod_size_text(cod_font* font, int* width, int* height, const char* text)
```

Determine how much space *text* will take to render in *font*. Returns
the dimensions in the *width* and *height* variables.

---

```c
void cod_draw_text(cod_font* font, const char* text, cod_pixel fg, cod_image* target, int dstx, int dsty)
```    

Draw *text* in *font* using the color *fg* at *dstx* and *dsty* in the image *target*.

#### Drawing primitives

Cod does not support primitive shapes and lines directly, but there are some
located in extra/drawing.c. Include in your code and link -lm on
Unix systems.
