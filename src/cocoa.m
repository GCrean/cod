// cocoa.m - cocoa support
#if COD_PLATFORM == COD_COCOA

#import <Cocoa/Cocoa.h>
#import <Foundation/NSString.h>

#include "cod.h"

@interface CodView : NSView<NSWindowDelegate> {

}
- (void) drawRect: (NSRect) rect;
@end // CodView

@implementation CodView
-(void) drawRect: (NSRect) rect {
  
}

-(void) windowWillClose: (NSNotification*) notification {
  printf("WEEe\n");
  [NSApp terminate: self];
}
@end // CodView

static NSWindow* window = 0;
static CodView* view = 0;
static NSAutoreleasePool* pool = 0;

int _cod_open() {
  // create window
  static unsigned style = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;

  pool = [[NSAutoreleasePool alloc] init];

  NSApp = [NSApplication sharedApplication];

  NSRect window_rect = NSMakeRect(5, 5, cod_window_width, cod_window_height);

  window = [[NSWindow alloc]
             initWithContentRect: window_rect
             styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
             backing: NSBackingStoreBuffered
             defer: NO];

  // window configuration
  [window setIgnoresMouseEvents: NO];
  [window setAcceptsMouseMovedEvents: YES];
  [window setReleasedWhenClosed: NO];
  [window setOneShot: NO];
  [window setCanHide: YES];
  [window setIsVisible: YES];

  // initialize instance of view (handles actual graphics)
  view = [[CodView alloc] initWithFrame: window_rect];

  [window setDelegate: view];
  [window setContentView: view];

  // display window
  [window makeKeyAndOrderFront: nil];

  [NSApp finishLaunching];
  [NSApp activateIgnoringOtherApps: YES];

  return 1;
}

void _cod_close() {
  [window release];
  view = 0;
  window = 0;
  [NSApp release];
  [pool release];
  pool = 0;
}

void cod_swap() {

}

void cod_set_title(const char* title) {
  NSString* string = [[NSString alloc] initWithUTF8String: title];
  [window setTitle: string];
  [string release];
}

int cod_get_event(cod_event* e) {
  NSEvent* event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode:NSDefaultRunLoopMode dequeue: YES];
  switch([event type]) {
    case NSKeyDown:
      break;
    default:
      [NSApp sendEvent: event];
      break;
  }
  [event release];
  return 0;
}

extern int usleep(unsigned int);

void cod_sleep(int milliseconds) {
  usleep(milliseconds * 1000);
}

#endif // COD_PLATFORM == COD_COCOA
