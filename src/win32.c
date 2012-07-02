// win32.c - cod windows backend
#if COD_PLATFORM == COD_WIN32

#define UNICODE 1

#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>

#include "cod.h"

#pragma comment(lib, "user32.lib")
// Needed for CreateDIBSection
#pragma comment(lib, "gdi32.lib")

static HWND window = NULL;
// Not really clear on whether we can just grab this from GetDC or
// if we have to use ReleaseDC on it every time we call GetDC, so we'll
// just going to cache it here and ReleaseDC at the end of its lifetime
static HDC window_hdc = NULL;

// Framebuffer
static HBITMAP buffer_bitmap = NULL;
static unsigned char* buffer_data = NULL;
static HDC buffer_hdc = NULL;

extern int main(int argc, char** argv);

// This is just a barebones WndProc to ensure the window is created
// properly (windows will not accept a NULL WndProc) and that WM_CLOSE
// is intercepted (apparently PeekMessage will not find WM_CLOSE for
// some reason)
LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch(msg) {
    case WM_CREATE: 
      return 0;
    case WM_PAINT:
      DefWindowProc(wnd, msg, wparam, lparam);
      return 0;
    case WM_CLOSE:
      // Intercept WM_CLOSE messages and send them to the normal
      // event-handling loop (for some reason, only the window's
      // WndProc receives WM_CLOSE and PeekMessage won't find it)
      PostMessage(NULL, WM_CLOSE, wparam, lparam);
      break;
    default:
      return DefWindowProc(wnd, msg, wparam, lparam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {
#if COD_WINDOWS_DEBUG
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  freopen("CON", "w", stdout);
#endif

  return main(0, NULL);
}

// Check for and report Windows errors
static void winfail() {
  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError(); 

  if(dw==0) return;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL);

  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
                                    (lstrlen((LPCTSTR)lpMsgBuf) + sizeof(TCHAR)));
  StringCchPrintf((LPTSTR)lpDisplayBuf, 
                  LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                  TEXT("failed with error %d: %s"), 
                  dw, lpMsgBuf); 
  MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}

int _cod_open() {
  BITMAPINFO bi;
  WNDCLASSEX window_class;
  HINSTANCE hinstance = GetModuleHandle(NULL);
	LPCTSTR window_class_name = TEXT("cod");

  ZeroMemory(&window_class, sizeof(WNDCLASSEX));
  window_class.cbSize = sizeof(WNDCLASSEX);
  window_class.style = 0;
  window_class.lpfnWndProc = WndProc;
  window_class.hInstance = hinstance;
  // No wacky cursors
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  // No icon, background, or menu name

  window_class.lpszClassName = window_class_name;

  if(!RegisterClassEx(&window_class)) {
		winfail();
    COD_ERROR0("cod_open: win32: RegisterClassEx failed");
    return 0;
  }

  window = CreateWindow
    (window_class_name, window_class_name, 
     (WS_OVERLAPPEDWINDOW | WS_SYSMENU) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME),
     //     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
     // Position
     0, 0,
     // Dimensions
     cod_window_width, cod_window_height, 
     NULL, // No parent
     NULL, // No menu
     hinstance,
     NULL);

  if(!window) {
		winfail();
    COD_ERROR0("cod_open: win32: CreateWindowEx failed");
    return 0;
  }

  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);

  window_hdc = GetDC(window);
  // Create a bitmap we can write to 
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biWidth = cod_window_width;
  bi.bmiHeader.biHeight = -cod_window_height; 
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 32;
  bi.bmiHeader.biCompression = BI_RGB;
  bi.bmiHeader.biSizeImage = 0;
  bi.bmiHeader.biClrUsed = 0;
  bi.bmiHeader.biClrImportant = 0;
  buffer_bitmap = CreateDIBSection(window_hdc, &bi, DIB_RGB_COLORS, (void**) &buffer_data, NULL, 0);

  if(!buffer_bitmap) {
    winfail();
    COD_ERROR0("cod_open: win32: CreateDIBSection failed");
    return 0;
  }

  buffer_hdc = CreateCompatibleDC(window_hdc);
  SelectObject(buffer_hdc, buffer_bitmap);


  return 1;
}

void _cod_close() {
  if(buffer_bitmap) DeleteObject(buffer_bitmap);
  if(buffer_hdc) DeleteDC(buffer_hdc);
  if(window && window_hdc) ReleaseDC(window, window_hdc);
  if(window) DestroyWindow(window);

  buffer_bitmap = NULL;
  buffer_data = NULL;
  buffer_hdc = NULL;
  window_hdc = NULL;
  window = NULL;
}

void cod_set_title(const char* title) {
  SetWindowTextA(window, title);
}

void cod_swap() {
  int x, y, offset, w_offset;

  for(x = 0; x < cod_window_width; x++) {
    for(y = 0; y < cod_window_height; y++) {
      offset = (y * cod_window_width) + x;
      w_offset = offset * 4;

      buffer_data[w_offset] = COD_PIXEL_B(cod_screen->data[offset]);
      buffer_data[w_offset+1] = COD_PIXEL_G(cod_screen->data[offset]);
      buffer_data[w_offset+2] = COD_PIXEL_R(cod_screen->data[offset]);
      buffer_data[w_offset+3] = COD_PIXEL_A(cod_screen->data[offset]);
    }
  }
  
  BitBlt(window_hdc, 0, 0, cod_window_width, cod_window_height, buffer_hdc, 0, 0, SRCCOPY);
}

void cod_sleep(int milliseconds) { 
  Sleep(milliseconds);
}

///// EVENT HANDLING

// This could probably use an array to translate, with a little bounds checking
// of course
static cod_key translate_key(unsigned char key, MSG* msg, int keyup) {
  switch(key) {
#define _(a,b) case (a): return COD_KEY_##b; 
    _(VK_RETURN, ENTER);
    _(VK_OEM_COMMA, COMMA);
    _(VK_OEM_PERIOD, PERIOD);
    _(VK_OEM_PLUS, EQUAL);
    _(VK_OEM_MINUS, MINUS);
    _(VK_OEM_1, SEMICOLON);
    _(VK_OEM_2, SLASH);
    _(VK_OEM_3, GRAVE);
    _(VK_OEM_4, LEFT_BRACKET);
    _(VK_OEM_5, BACKSLASH);
    _(VK_OEM_6, RIGHT_BRACKET); 
    _(VK_OEM_7, APOSTROPHE);
    _(VK_NUMPAD0, NUMPAD_0);
    _(VK_NUMPAD1, NUMPAD_1);
    _(VK_NUMPAD2, NUMPAD_2);
    _(VK_NUMPAD3, NUMPAD_3);
    _(VK_NUMPAD4, NUMPAD_4);
    _(VK_NUMPAD5, NUMPAD_5);
    _(VK_NUMPAD6, NUMPAD_6);
    _(VK_NUMPAD7, NUMPAD_7);
    _(VK_NUMPAD8, NUMPAD_8);
    _(VK_NUMPAD9, NUMPAD_9);
    _(VK_BACK, BACKSPACE);
    _(VK_LEFT, LEFT_ARROW);
    _(VK_RIGHT, RIGHT_ARROW);
    _(VK_DOWN, DOWN_ARROW);
    _(VK_UP, UP_ARROW);
    _(VK_TAB, TAB);
    _(VK_SPACE, SPACE);
    _(VK_ESCAPE, ESCAPE);
    _(VK_LWIN, SUPER_L);
    _('0', 0);
    _('1', 1);
    _('2', 2);
    _('3', 3);
    _('4', 4);
    _('5', 5);
    _('6', 6);
    _('7', 7);
    _('8', 8);
    _('9', 9);
    _('A', A);
    _('B', B);
    _('C', C);
    _('D', D);
    _('E', E);
    _('F', F);
    _('G', G);
    _('H', H);
    _('I', I);
    _('J', J);
    _('K', K);
    _('L', L);
    _('M', M);
    _('N', N);
    _('O', O);
    _('P', P);
    _('Q', Q);
    _('R', R);
    _('S', S);
    _('T', T);
    _('U', U);
    _('V', V);
    _('W', W);
    _('X', X);
    _('Y', Y);
    _('Z', Z);
    case VK_F1: return COD_KEY_F1;
    case VK_F2: return COD_KEY_F2;
    case VK_F3: return COD_KEY_F3;
    case VK_F4: return COD_KEY_F4;
    case VK_F5: return COD_KEY_F5;
    case VK_F6: return COD_KEY_F6;
    case VK_F7: return COD_KEY_F7;
    case VK_F8: return COD_KEY_F8;
    case VK_F9: return COD_KEY_F9;
    case VK_F10: return COD_KEY_F10;
    case VK_F11: return COD_KEY_F11;
    case VK_F12: return COD_KEY_F12;
    case VK_F13: return COD_KEY_F13;
    case VK_F14: return COD_KEY_F14;
    case VK_F15: return COD_KEY_F15;
    case VK_INSERT: return COD_KEY_INSERT;
    case VK_DELETE: return COD_KEY_DELETE;
    case VK_HOME: return COD_KEY_HOME;
    case VK_PRIOR: return COD_KEY_PAGE_UP;
    case VK_NEXT: return COD_KEY_PAGE_DOWN;
    case VK_END: return COD_KEY_END;

#define EXTENDED_KEYMASK (1<<24)
    case VK_SHIFT:
      // This is bizarre. And it STILL doesn't work correctly if you
      // hold down right and then left shift. Hopefully this won't
      // become a problem later...
      if(keyup) {
        int scan_l = MapVirtualKey(VK_LSHIFT, 0);
        int scan_r = MapVirtualKey(VK_RSHIFT, 0);
        int key_code = (int)((msg->lParam & 0xff0000) >> 16);
        if(key_code == scan_r)
          return COD_KEY_RIGHT_SHIFT;
        return COD_KEY_LEFT_SHIFT;
      } else {
        if(GetKeyState(VK_RSHIFT) & 0x80) return COD_KEY_RIGHT_SHIFT;
        return COD_KEY_LEFT_SHIFT;
      }
    case VK_CONTROL:
      if(msg->lParam & EXTENDED_KEYMASK) return COD_KEY_RIGHT_CONTROL;
      return COD_KEY_LEFT_CONTROL;
    case VK_MENU:
      if(msg->lParam & EXTENDED_KEYMASK) return COD_KEY_RIGHT_ALT;
      return COD_KEY_LEFT_ALT;
#undef EXTENDED_KEYMASK
#undef _
  }
  return COD_KEY_UNKNOWN;
}

int cod_get_event(cod_event* cevent) {
  MSG msg;

  // PeekMessage is in a loop so we can skip messages with "continue"
  // but still process the message queue till we get to the end or a
  // message we actually care about
  while(1) {
    // Check whether a message is waiting to be processed,
    // if there is, remove it from the queue so we can use it.
    // No filtering or window-specific messages are required so we leave those
    // parameters as 0
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      // In a normal Windows program, we'd use DispatchMessage here, but we're
      // using our own custom dispatching, so we just translate to Cod events and
      // give control back to the main loop
      switch(msg.message) {
        case WM_SETFOCUS:
          cevent->type = COD_FOCUS;
          DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
          return 1;
        case WM_KILLFOCUS:
          cevent->type = COD_UNFOCUS;
          DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
          return 1;
        case WM_PAINT:
          cevent->type = COD_REDRAW;
          DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
          return 1;
          // WM_SETCURSOR is called for every WM_MOUSEMOVE? 
        case WM_SETCURSOR:
          continue;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: 
        case WM_KEYUP:
        case WM_SYSKEYUP: {
          unsigned char key = (unsigned char) msg.wParam;
          cevent->type = (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) ? COD_KEY_DOWN : COD_KEY_UP;
          cevent->key_down.key = translate_key(key, &msg, cevent->type == COD_KEY_UP);
          cevent->key_down.modifiers = 0;
          if(GetKeyState(VK_CONTROL) < 0) cevent->key_down.modifiers |= COD_MOD_CONTROL;
          if(GetKeyState(VK_MENU) < 0) cevent->key_down.modifiers |= COD_MOD_ALT;
          if(GetKeyState(VK_SHIFT) < 0) cevent->key_down.modifiers |= COD_MOD_SHIFT;
          return 1;
        }
          // Mouse events
        case WM_MOUSEMOVE:
          cevent->type = COD_MOUSE_MOTION;
          cevent->mouse_motion.x = GET_X_LPARAM(msg.lParam);
          cevent->mouse_motion.y = GET_Y_LPARAM(msg.lParam);
          return 1;

#define mevent(_type, _key) cevent->type = (_type); cevent->key_down.key = (_key); cevent->key_down.x = GET_X_LPARAM(msg.lParam); cevent->key_down.y = GET_Y_LPARAM(msg.lParam); return 1;
        case WM_MBUTTONUP: mevent(COD_KEY_UP, COD_MOUSE_MIDDLE);
        case WM_RBUTTONUP: mevent(COD_KEY_UP, COD_MOUSE_RIGHT);
        case WM_LBUTTONUP: mevent(COD_KEY_UP, COD_MOUSE_LEFT);
        case WM_MBUTTONDOWN: mevent(COD_KEY_DOWN, COD_MOUSE_MIDDLE);
        case WM_RBUTTONDOWN: mevent(COD_KEY_DOWN, COD_MOUSE_RIGHT);
        case WM_LBUTTONDOWN: mevent(COD_KEY_DOWN, COD_MOUSE_LEFT);
#undef mevent
          // WM_CLOSE means the user has asked for the window to be closed
          // Calling DefWindowProc will destroy the window
        case WM_CLOSE:
          DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
          DestroyWindow(msg.hwnd);
          cevent->type = COD_QUIT;
          return 1;
          // WM_DESTROY means the window has been destroyed
        case WM_DESTROY:
        case WM_NCDESTROY:
        case WM_QUIT:
        default:
          DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
          break;
      }
    }
    break;
  }
  return 0;
}

#endif // COD_PLATFORM == COD_WIN32
