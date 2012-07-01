// cocoa.m - cocoa support
#if COD_PLATFORM == COD_COCOA

#import <Cocoa/Cocoa.h>
#import <Foundation/NSString.h>
#import <Carbon/Carbon.h> // key codes

#include <math.h>

#include "cod.h"

// Declare setAppleMenu
@interface CodView : NSView<NSWindowDelegate> {

}
- (void) drawRect: (NSRect) rect;
@end // CodView

static NSWindow* window = 0;
static CodView* view = 0;
static NSAutoreleasePool* pool = 0;
static NSBitmapImageRep* cocoa_pixels = 0;
static char received_terminate = 0, sent_quit = 0, received_draw = 1;

@implementation CodView
-(void) drawRect: (NSRect) rect {
  NSLog(@"drawRect called");
  received_draw = 1;
  if(!received_terminate) {
    [NSGraphicsContext saveGraphicsState];
    [cocoa_pixels drawInRect: rect];
    [NSGraphicsContext restoreGraphicsState];
  }
}

-(void) windowWillClose: (NSNotification*) notification {
  received_terminate = 1;
}
@end // CodView


static cod_key translate_key(unsigned);

int _cod_open() {

  pool = [[NSAutoreleasePool alloc] init];
  NSApp = [NSApplication sharedApplication];
  
  // change activation policy so that applications without bundles can be brought to the front
  // this only took me about 5 hours to figure out...
  [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

  // barebones menu
  id menubar = [[NSMenu new] autorelease];
  id appMenuItem = [[NSMenuItem new] autorelease];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];
  id appMenu = [[NSMenu new] autorelease];
  id appName = [[NSProcessInfo processInfo] processName];
  id quitTitle = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
      action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
  [appMenu addItem:quitMenuItem];
  [appMenuItem setSubmenu:appMenu];
  
    // create window  
  NSRect rect = NSMakeRect(0, 0, cod_window_width, cod_window_height);

  window = [[NSWindow alloc]
    initWithContentRect: rect
              styleMask: NSTitledWindowMask | NSClosableWindowMask
                backing: NSBackingStoreBuffered
                  defer: NO];
  
  [window setTitle:@"cod application"];

  // set up window
  view = [[CodView alloc] initWithFrame: rect];

  [window setContentView: view];
  [window setDelegate: view];

  // configure window
  [window setAcceptsMouseMovedEvents: YES];

  // make window visible
  [window makeKeyAndOrderFront: nil];

  [NSApp finishLaunching];
  [NSApp activateIgnoringOtherApps: YES];

  cocoa_pixels = [[NSBitmapImageRep alloc]
    initWithBitmapDataPlanes: NULL
                 pixelsWide: cod_window_width
                 pixelsHigh: cod_window_height
              bitsPerSample: 8
            samplesPerPixel: 4
                   hasAlpha: YES
                   isPlanar: NO
             colorSpaceName: NSDeviceRGBColorSpace
                bytesPerRow: 0
               bitsPerPixel: 0];
  
  return 1;
}

void _cod_close() {
  received_terminate = sent_quit = 0;
  [NSApp terminate: view];
  [view release];
  [window release];
  [NSApp release];
  [pool release];
}

void cod_swap() {
  if(!received_terminate) {
    [pool release];
    pool = [[NSAutoreleasePool alloc] init];

    int y, x, offset, coffset;
    unsigned char* pixels = (unsigned char*) [cocoa_pixels bitmapData];

  
    for(y = 0; y < cod_window_height; y++) {
      for(x = 0; x < cod_window_width; x++) {
        offset = (y * cod_window_width) + x;
        coffset = offset * 4;
        pixels[coffset] = COD_PIXEL_R(cod_screen->data[offset]);
        pixels[coffset+1] = COD_PIXEL_G(cod_screen->data[offset]);
        pixels[coffset+2] = COD_PIXEL_B(cod_screen->data[offset]);
        pixels[coffset+3] = COD_PIXEL_A(cod_screen->data[offset]);
      }
    }

    [NSGraphicsContext saveGraphicsState];
    [cocoa_pixels drawInRect: [view bounds]];
    [NSGraphicsContext restoreGraphicsState];
  }
}

void cod_set_title(const char* title) {
  NSString* string = [[NSString alloc] initWithUTF8String: title];
  [window setTitle: string];
  [string release];
}

int cod_get_event(cod_event* e) {
  [pool release];
  pool = [[NSAutoreleasePool alloc] init];
  
  if(received_terminate && !sent_quit) {
    e->type = COD_QUIT;
    sent_quit = 1;
    return 1;
  } else if(sent_quit) {
    return 0;
  }

  if(received_draw) {
    e->type = COD_REDRAW;
    received_draw = 0;
    return 1;
  }

  NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode:NSDefaultRunLoopMode dequeue: YES];
  switch([event type]) {
    case NSKeyDown:
      // Pass through commands
      if([event modifierFlags] & NSCommandKeyMask) {
        [NSApp sendEvent: event];
      }
    case NSKeyUp:
      e->type = [event type] == NSKeyDown ? COD_KEY_DOWN : COD_KEY_UP;
      e->data.key_down.key = translate_key([event keyCode]);
      return 1;
      break;
#define mevent(type_, key_) {                                     \
        e->type = (type_);                                        \
        e->data.key_down.key = (key_);                            \
        NSPoint point = [event locationInWindow];                 \
        if((cod_window_height - (int)point.y) < 0) {              \
          [NSApp sendEvent: event];                               \
          break;                                                  \
        } else {                                                  \
          e->data.key_down.x = (int) point.x;                     \
          e->data.key_down.y = cod_window_height - (int) point.y; \
          return 1;                                               \
        }                                                         \
    }
    // Mouse press
    case NSRightMouseDown: mevent(COD_KEY_DOWN, COD_MOUSE_RIGHT); 
    case NSRightMouseUp: mevent(COD_KEY_UP, COD_MOUSE_RIGHT); 
    case NSLeftMouseUp: mevent(COD_KEY_UP, COD_MOUSE_LEFT); 
    case NSLeftMouseDown: mevent(COD_KEY_DOWN, COD_MOUSE_LEFT); 
    default:
      [NSApp sendEvent: event];
      break;
  }

  [NSApp sendEvent: event];
  [NSApp updateWindows];

  return 0;
}

extern int usleep(unsigned int);

void cod_sleep(int milliseconds) {
  usleep(milliseconds * 1000);
}

static cod_key translate_key(unsigned key) {
  switch(key) {
    case kVK_ANSI_A: return COD_KEY_A;
    case kVK_ANSI_B: return COD_KEY_B;
    case kVK_ANSI_C: return COD_KEY_C;
    case kVK_ANSI_D: return COD_KEY_D;
    case kVK_ANSI_E: return COD_KEY_E;
    case kVK_ANSI_F: return COD_KEY_F;
    case kVK_ANSI_G: return COD_KEY_G;
    case kVK_ANSI_H: return COD_KEY_H;
    case kVK_ANSI_I: return COD_KEY_I;
    case kVK_ANSI_J: return COD_KEY_J;
    case kVK_ANSI_K: return COD_KEY_K;
    case kVK_ANSI_L: return COD_KEY_L;
    case kVK_ANSI_M: return COD_KEY_M;
    case kVK_ANSI_N: return COD_KEY_N;
    case kVK_ANSI_O: return COD_KEY_O;
    case kVK_ANSI_P: return COD_KEY_P;
    case kVK_ANSI_Q: return COD_KEY_Q;
    case kVK_ANSI_R: return COD_KEY_R;
    case kVK_ANSI_S: return COD_KEY_S;
    case kVK_ANSI_T: return COD_KEY_T;
    case kVK_ANSI_U: return COD_KEY_U;
    case kVK_ANSI_V: return COD_KEY_V;
    case kVK_ANSI_W: return COD_KEY_W;
    case kVK_ANSI_X: return COD_KEY_X;
    case kVK_ANSI_Y: return COD_KEY_Y;
    case kVK_ANSI_Z: return COD_KEY_Z;
    case kVK_ANSI_0: return COD_KEY_0;
    case kVK_ANSI_1: return COD_KEY_1;
    case kVK_ANSI_2: return COD_KEY_2;
    case kVK_ANSI_3: return COD_KEY_3;
    case kVK_ANSI_4: return COD_KEY_4;
    case kVK_ANSI_5: return COD_KEY_5;
    case kVK_ANSI_6: return COD_KEY_6;
    case kVK_ANSI_7: return COD_KEY_7;
    case kVK_ANSI_8: return COD_KEY_8;
    case kVK_ANSI_9: return COD_KEY_9;
    case kVK_ANSI_Equal: return COD_KEY_EQUAL; break;
    case kVK_ANSI_Minus: return COD_KEY_MINUS; break;
    case kVK_ANSI_RightBracket: return COD_KEY_RIGHT_BRACKET; break;
    case kVK_ANSI_LeftBracket: return COD_KEY_LEFT_BRACKET; break;
    case kVK_ANSI_Quote: return COD_KEY_APOSTROPHE; break;
    case kVK_ANSI_Semicolon: return COD_KEY_SEMICOLON; break;
    case kVK_ANSI_Backslash: return COD_KEY_BACKSLASH; break;
    case kVK_ANSI_Comma: return COD_KEY_COMMA; break;
    case kVK_ANSI_Slash: return COD_KEY_SLASH; break;
    case kVK_ANSI_Period: return COD_KEY_PERIOD; break;
    case kVK_ANSI_Grave: return COD_KEY_GRAVE; break;
    case kVK_ANSI_Keypad0: return COD_KEY_NUMPAD_0; break;
    case kVK_ANSI_Keypad1: return COD_KEY_NUMPAD_1; break;
    case kVK_ANSI_Keypad2: return COD_KEY_NUMPAD_2; break;
    case kVK_ANSI_Keypad3: return COD_KEY_NUMPAD_3; break;
    case kVK_ANSI_Keypad4: return COD_KEY_NUMPAD_4; break;
    case kVK_ANSI_Keypad5: return COD_KEY_NUMPAD_5; break;
    case kVK_ANSI_Keypad6: return COD_KEY_NUMPAD_6; break;
    case kVK_ANSI_Keypad7: return COD_KEY_NUMPAD_7; break;
    case kVK_ANSI_Keypad8: return COD_KEY_NUMPAD_8; break;
    case kVK_ANSI_Keypad9: return COD_KEY_NUMPAD_9; break;
    case kVK_F1: return COD_KEY_F1;
    case kVK_F2: return COD_KEY_F2;
    case kVK_F3: return COD_KEY_F3;
    case kVK_F4: return COD_KEY_F4;
    case kVK_F5: return COD_KEY_F5;
    case kVK_F6: return COD_KEY_F6;
    case kVK_F7: return COD_KEY_F7;
    case kVK_F8: return COD_KEY_F8;
    case kVK_F9: return COD_KEY_F9;
    case kVK_F10: return COD_KEY_F10;
    case kVK_F11: return COD_KEY_F11;
    case kVK_F12: return COD_KEY_F12;
    case kVK_F13: return COD_KEY_F13;
    case kVK_F14: return COD_KEY_F14;
    case kVK_F15: return COD_KEY_F15;
    case kVK_Return: return COD_KEY_ENTER; break;
    case kVK_Tab: return COD_KEY_TAB; break;
    case kVK_Space: return COD_KEY_SPACE; break;
    case kVK_Delete: return COD_KEY_BACKSPACE; break;
    case kVK_Escape: return COD_KEY_ESCAPE; break;
    case kVK_Command: return COD_KEY_SUPER_L; break;
    case kVK_Shift: return COD_KEY_LEFT_SHIFT; break;
    case kVK_CapsLock: return COD_KEY_CAPS_LOCK; break;
    case kVK_Option: return COD_KEY_LEFT_ALT; break;
    case kVK_Control: return COD_KEY_LEFT_CONTROL; break;
    case kVK_RightShift: return COD_KEY_RIGHT_SHIFT; break;
    case kVK_RightOption: return COD_KEY_RIGHT_ALT; break;
    case kVK_RightControl: return COD_KEY_RIGHT_CONTROL; break;
    case kVK_LeftArrow: return COD_KEY_LEFT_ARROW; break;
    case kVK_RightArrow: return COD_KEY_RIGHT_ARROW; break;
    case kVK_DownArrow: return COD_KEY_DOWN_ARROW; break;
    case kVK_UpArrow: return COD_KEY_UP_ARROW; break;
    default: return COD_KEY_UNKNOWN;
  }
  return COD_KEY_UNKNOWN;
}

#endif // COD_PLATFORM == COD_COCOA
