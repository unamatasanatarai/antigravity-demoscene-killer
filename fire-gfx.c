/**
 * fire-gfx.c - High-Performance Graphical Fire Simulation (macOS Cocoa)
 *
 * A standalone, single-file Cocoa application implementing the classic Doom
 * fire algorithm. Renders directly to a pixel buffer and displays it in a
 * native window.
 *
 * Compile with:
 *   clang -O3 -x objective-c -framework Cocoa fire-gfx.c -o fire-gfx
 */

#import <Cocoa/Cocoa.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// --- Configuration ---
#define FIRE_WIDTH 320
#define FIRE_HEIGHT 200
#define SCALE 3
#define WINDOW_WIDTH (FIRE_WIDTH * SCALE)
#define WINDOW_HEIGHT (FIRE_HEIGHT * SCALE)
#define FPS 60

// --- Globals ---
static uint8_t fire_buffer[FIRE_WIDTH * FIRE_HEIGHT];
static uint32_t pixel_buffer[FIRE_WIDTH * FIRE_HEIGHT]; // ARGB
static uint32_t palette[256];

// --- Fire Algorithm ---

void init_palette(void) {
  for (int i = 0; i < 256; i++) {
    // HSL-like color generation for fire: Black -> Red -> Orange -> Yellow ->
    // White We'll manually interpolate for speed and control.
    uint8_t r = 0, g = 0, b = 0;

    if (i < 64) {
      r = i * 4;
      g = 0;
      b = 0;
    } else if (i < 128) {
      r = 255;
      g = (i - 64) * 4;
      b = 0;
    } else if (i < 192) {
      r = 255;
      g = 255;
      b = (i - 128) * 4;
    } else {
      r = 255;
      g = 255;
      b = 255;
    }

    // ARGB format (Little Endian: B G R A)
    // But CGImage usually wants premultiplied or simple RGB.
    // Let's use kCGImageAlphaNoneSkipFirst: XRGB.
    // So 0x00RRGGBB
    palette[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
  }
}

void update_fire(void) {
  // 1. Seed bottom row
  int last_row = (FIRE_HEIGHT - 1) * FIRE_WIDTH;
  for (int x = 0; x < FIRE_WIDTH; x++) {
    if ((rand() % 100) < 60) {
      fire_buffer[last_row + x] = 255 - (rand() % 50);
    } else {
      if (fire_buffer[last_row + x] > 10)
        fire_buffer[last_row + x] -= 5;
    }
  }

  // 2. Propagate
  for (int y = 0; y < FIRE_HEIGHT - 1; y++) {
    for (int x = 0; x < FIRE_WIDTH; x++) {
      int src_idx = (y + 1) * FIRE_WIDTH + x;
      int val = fire_buffer[src_idx];

      if (val == 0) {
        fire_buffer[y * FIRE_WIDTH + x] = 0;
      } else {
        int decay = rand() % 3;
        int dst_x = x - (rand() % 3) + 1;
        if (dst_x < 0)
          dst_x = 0;
        if (dst_x >= FIRE_WIDTH)
          dst_x = FIRE_WIDTH - 1;

        int dst_idx = y * FIRE_WIDTH + dst_x;
        int new_val = val - decay;
        if (new_val < 0)
          new_val = 0;

        fire_buffer[dst_idx] = new_val;
      }
    }
  }

  // 3. Render to pixels
  for (int i = 0; i < FIRE_WIDTH * FIRE_HEIGHT; i++) {
    pixel_buffer[i] = palette[fire_buffer[i]];
  }
}

// --- Cocoa UI ---

@interface FireView : NSView
@end

@implementation FireView
- (void)drawRect:(NSRect)dirtyRect {
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];

  // Create CGImage from pixel buffer
  CGDataProviderRef provider = CGDataProviderCreateWithData(
      NULL, pixel_buffer, sizeof(pixel_buffer), NULL);
  CGImageRef image = CGImageCreate(
      FIRE_WIDTH, FIRE_HEIGHT, 8, 32, FIRE_WIDTH * 4, colorSpace,
      kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipFirst, // XRGB
      provider, NULL, false, kCGRenderingIntentDefault);

  // Draw scaled
  CGContextSetInterpolationQuality(ctx,
                                   kCGInterpolationNone); // Keep pixels sharp
  CGContextDrawImage(ctx, CGRectMake(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), image);

  CGImageRelease(image);
  CGDataProviderRelease(provider);
  CGColorSpaceRelease(colorSpace);
}
@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property(strong) NSWindow *window;
@property(strong) FireView *view;
@property(strong) NSTimer *timer;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
  // Create Window
  NSRect frame = NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                     NSWindowStyleMaskMiniaturizable |
                     NSWindowStyleMaskResizable;
  self.window = [[NSWindow alloc] initWithContentRect:frame
                                            styleMask:style
                                              backing:NSBackingStoreBuffered
                                                defer:NO];
  [self.window setTitle:@"Fire Simulation"];
  [self.window center];
  [self.window setDelegate:self];

  // Create View
  self.view = [[FireView alloc] initWithFrame:frame];
  [self.window setContentView:self.view];
  [self.window makeKeyAndOrderFront:nil];
  [self.window setBackgroundColor:[NSColor blackColor]];

  // Init Fire
  srand((unsigned)time(NULL));
  init_palette();

  // Start Loop
  self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / FPS
                                                target:self
                                              selector:@selector(tick:)
                                              userInfo:nil
                                               repeats:YES];
}

- (void)tick:(NSTimer *)timer {
  update_fire();
  [self.view setNeedsDisplay:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  return YES;
}

@end

// --- Main ---

int main(int argc, const char *argv[]) {
  @autoreleasepool {
    NSApplication *app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    AppDelegate *delegate = [[AppDelegate alloc] init];
    [app setDelegate:delegate];

    [app run];
  }
  return 0;
}
