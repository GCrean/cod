// x11.c -- cod x11 implementation

#if COD_PLATFORM == COD_X11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include "cod.h"

static Display* display = 0;
static Window window = 0;
static GC gc;
static XImage* image;
static char window_title[COD_BUFFER_SIZE];
static unsigned char* x_pixels;

int cod_open(int width, int height) {
  _cod_open(width, height);

  // Acquire display information
  if((display = XOpenDisplay(0)) == NULL) {
    COD_ERROR0("cod_open: x11: XOpenDisplay failed");
    return 0;
  }

  int screen = DefaultScreen(display);
  Visual* visual = DefaultVisual(display, screen);
  if(!visual) {
    COD_ERROR0("cod_open: x11: DefaultVisual failed");
  }

  Window root = DefaultRootWindow(display);

  // Get display depth
  int display_depth = DefaultDepth(display, screen);

  // Create the window 

  // Put our window in the middle

  XSetWindowAttributes attributes;
  attributes.border_pixel = attributes.background_pixel = BlackPixel(display, screen);

  window = XCreateWindow(display, root, 0, 0, cod_window_width, cod_window_height, 0, CopyFromParent,
			 InputOutput, CopyFromParent, CWBackPixel | CWBorderPixel, 
			 &attributes);

  if(!window) {
    COD_ERROR0("cod_open: x11: XCreateWindow failed");
    return 0;
  }

  // Allow the window manager to delete this window
  Atom wm_delete_window;
  wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", True);

  if(!wm_delete_window) {
    COD_ERROR0("cod_open: x11: XInternAtom failed");
    return 0;
  }

  if(!XSetWMProtocols(display, window, &wm_delete_window, 1)) {
    COD_ERROR0("cod_open: x11: XSetWMProtocols failed");
    return 0;
  }

  // Force X11 to obey our size specifications
  XSizeHints* sizehints = XAllocSizeHints();
  sizehints->min_width = sizehints->max_width = cod_window_width;
  sizehints->min_height = sizehints->max_height = cod_window_height;
  sizehints->flags = PMaxSize | PMinSize | USPosition;
  XSetWMNormalHints(display, window, sizehints);
  XFree(sizehints);  

  // Create image that we will draw to
  gc = DefaultGC(display, screen);
  image = XCreateImage(display, CopyFromParent, display_depth, ZPixmap, 0, NULL,
		       cod_window_width, cod_window_height, 32, cod_window_width * COD_BYTES_PER_PIXEL);

  if(!image) {
    COD_ERROR0("cod_open: x11: XCreateImage failed");
    return 0;
  }

  cod_pixels = cod_make_image(cod_window_width, cod_window_height);
  x_pixels = COD_ALLOCATE(cod_window_width * cod_window_height * 4);  

  // Tell X11 what events we care about
  XSelectInput(display, window, 
	       KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask);

  // Map window and return
  XMapRaised(display, window);

  XFlush(display);

  return 1;
}

void cod_set_title(const char* title) {
  strncpy(window_title, title, COD_BUFFER_SIZE);
  XTextProperty title_property;
  // Clang generates a warning because window_title is char[512] and
  // not char*, this suppresses that
  char* title_ptr = (char*) window_title;
  XStringListToTextProperty(&title_ptr, 1, &title_property);
  XSetWMName(display, window, &title_property);
  XFree(title_property.value);
  XFlush(display);
}

void cod_close() {
  _cod_close();

  if(x_pixels) {
    free(x_pixels);
  }

  if(image) {
    XDestroyImage(image);
    image = 0;
  }

  if(display) {
    if(window) {
      XDestroyWindow(display, window);
    }
    XCloseDisplay(display);
    display = 0;
  }
}

static cod_key translate_button(int button) {
  switch(button) {
    case Button1: return COD_MOUSE_LEFT;
    case Button2: return COD_MOUSE_MIDDLE;
    case Button3: return COD_MOUSE_RIGHT;
    case Button4: return COD_MOUSE_WHEELUP;
    case Button5: return COD_MOUSE_WHEELDOWN;
    default: return COD_KEY_UNKNOWN;
  }
}

static void set_button(cod_event* cevent, XEvent* xevent) {
  cevent->data.key_down.x = xevent->xmotion.x;
  cevent->data.key_down.y = xevent->xmotion.y;
  cevent->data.key_down.key = translate_button(xevent->xbutton.button);

}

static cod_key translate_key(XEvent* xevent) {
  KeySym keysym = XLookupKeysym(&xevent->xkey, 0);
  switch(keysym) {
    case XK_a: return COD_KEY_A;
    case XK_b: return COD_KEY_B;
    case XK_c: return COD_KEY_C;
    case XK_d: return COD_KEY_D;
    case XK_e: return COD_KEY_E;
    case XK_f: return COD_KEY_F;
    case XK_g: return COD_KEY_G;
    case XK_h: return COD_KEY_H;
    case XK_i: return COD_KEY_I;
    case XK_j: return COD_KEY_J;
    case XK_k: return COD_KEY_K;
    case XK_l: return COD_KEY_L;
    case XK_m: return COD_KEY_M;
    case XK_n: return COD_KEY_N;
    case XK_o: return COD_KEY_O;
    case XK_p: return COD_KEY_P;
    case XK_q: return COD_KEY_Q;
    case XK_r: return COD_KEY_R;
    case XK_s: return COD_KEY_S;
    case XK_t: return COD_KEY_T;
    case XK_u: return COD_KEY_U;
    case XK_v: return COD_KEY_V;
    case XK_w: return COD_KEY_W;
    case XK_x: return COD_KEY_X;
    case XK_y: return COD_KEY_Y;
    case XK_0: return COD_KEY_0;
    case XK_1: return COD_KEY_1;
    case XK_2: return COD_KEY_2;
    case XK_3: return COD_KEY_3;
    case XK_4: return COD_KEY_4;
    case XK_5: return COD_KEY_5;
    case XK_6: return COD_KEY_6;
    case XK_7: return COD_KEY_7;
    case XK_8: return COD_KEY_8;
    case XK_9: return COD_KEY_9;
    case XK_Left: return COD_KEY_LEFT_ARROW;
    case XK_Up: return COD_KEY_UP_ARROW;
    case XK_Down: return COD_KEY_DOWN_ARROW;
    case XK_Right: return COD_KEY_RIGHT_ARROW;
#define _(a, b) case XK_##a: return COD_KEY_##b
      _(Control_L, LEFT_CONTROL);
      _(Control_R, RIGHT_CONTROL);
      _(Shift_L, LEFT_SHIFT);
      _(Shift_R, RIGHT_SHIFT);
      _(Alt_L, LEFT_ALT);
      // Doesnt seem to work, getting 65027, 0xfe07 or XK_ISO_Level3_Shift apparently
      _(Alt_R, RIGHT_ALT);
      _(Return, ENTER);
      _(BackSpace, BACKSPACE);
      _(semicolon, SEMICOLON);
      _(comma, COMMA);
      _(period, PERIOD);
      _(slash, SLASH);
      _(backslash, BACKSLASH);
      _(apostrophe, APOSTROPHE);
      _(bracketleft, LEFT_BRACKET);
      _(bracketright, RIGHT_BRACKET);
      _(minus, MINUS);
      _(equal, EQUAL);
      _(space, SPACE);
      _(Super_L, SUPER_L);
      _(Tab, TAB);
      _(Caps_Lock, CAPS_LOCK);
      _(grave, GRAVE);
#undef _
      
    default: return COD_KEY_UNKNOWN;
  }

}

int cod_get_event(cod_event* cevent) {
  memset(cevent, 0, sizeof(cod_event));
  XEvent xevent;

  if(XPending(display)) {
    XNextEvent(display, &xevent);
    switch(xevent.type) {
      case ButtonPress:
        cevent->type = COD_KEY_DOWN;
        set_button(cevent, &xevent);
        return 1;
      case ButtonRelease:
        cevent->type = COD_KEY_UP;
        set_button(cevent, &xevent);
        return 1;
      case KeyPress:
        cevent->type = COD_KEY_DOWN;
        cevent->data.key_down.key = translate_key(&xevent);
        return 1;
      case KeyRelease:
        cevent->type = COD_KEY_UP;
        cevent->data.key_down.key = translate_key(&xevent);
        return 1;
      case ClientMessage:
        cevent->type = COD_QUIT;
        return 1;
      default:
        break;
    }
  }

  return 0;
}

void cod_swap() {
  // Convert RGBA to BGRA for x (cause apparently that's what it wants?), then write to screen
  for(int x = 0; x < cod_window_width; x++) {
    for(int y = 0; y < cod_window_height; y++) {
      int cod_offset = (y * cod_window_width) + x;
      int x_offset = cod_offset * 4;
      x_pixels[x_offset] = cod_pixels->data[cod_offset].b;
      x_pixels[x_offset+1] = cod_pixels->data[cod_offset].g;
      x_pixels[x_offset+2] = cod_pixels->data[cod_offset].r;
      x_pixels[x_offset+3] = cod_pixels->data[cod_offset].a;
    }
  }

  image->data = (char*) x_pixels;

  XPutImage(display, window, gc, image, 0, 0, 0, 0, cod_window_width, cod_window_height);
  XFlush(display);

  image->data = NULL;
}

extern int usleep(int);

void cod_sleep(int seconds) {
  usleep(seconds);
}

#endif // COD_PLATFORM == COD_X11
