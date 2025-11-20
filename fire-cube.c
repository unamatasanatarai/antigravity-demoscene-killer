/**
 * fire-cube.c - 3D Fire Cube Simulation (macOS Cocoa + OpenGL)
 *
 * A standalone, single-file Cocoa application rendering the classic Doom fire
 * as a texture on a rotating 3D cube.
 *
 * Compile with:
 *   clang -O3 -x objective-c -framework Cocoa -framework OpenGL fire-cube.c -o
 * fire-cube
 */

#define GL_SILENCE_DEPRECATION // Silence OpenGL deprecation warnings on macOS

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// --- Configuration ---
#define FIRE_WIDTH 128
#define FIRE_HEIGHT 128
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 60

// --- Globals ---
static uint8_t fire_buffer[FIRE_WIDTH * FIRE_HEIGHT];
static uint32_t pixel_buffer[FIRE_WIDTH * FIRE_HEIGHT]; // ARGB
static uint32_t palette[256];
static GLuint fire_texture;
static float rot_x = 0.0f;
static float rot_y = 0.0f;
static float rot_z = 0.0f;

// --- Fire Algorithm ---

void init_palette(void) {
  for (int i = 0; i < 256; i++) {
    uint8_t r = 0, g = 0, b = 0;
    if (i < 64) {
      r = i * 4;
    } else if (i < 128) {
      r = 255;
      g = (i - 64) * 4;
    } else if (i < 192) {
      r = 255;
      g = 255;
      b = (i - 128) * 4;
    } else {
      r = 255;
      g = 255;
      b = 255;
    }
    // OpenGL expects RGBA or BGRA. Let's use GL_BGRA.
    // 0xAARRGGBB
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

// --- OpenGL View ---

@interface FireGLView : NSOpenGLView
@end

@implementation FireGLView

- (void)prepareOpenGL {
  [super prepareOpenGL];

  // Init Texture
  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &fire_texture);
  glBindTexture(GL_TEXTURE_2D, fire_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_NEAREST); // Blocky look is cool

  glEnable(GL_DEPTH_TEST);

  // Setup projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // Simple perspective
  float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
  float fov = 60.0f;
  float near = 0.1f;
  float far = 100.0f;
  float top = tan(fov * M_PI / 360.0f) * near;
  float right = top * aspect;
  glFrustum(-right, right, -top, top, near, far);

  glMatrixMode(GL_MODELVIEW);
}

- (void)drawRect:(NSRect)dirtyRect {
  [[self openGLContext] makeCurrentContext];

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -3.0f);
  glRotatef(rot_x, 1.0f, 0.0f, 0.0f);
  glRotatef(rot_y, 0.0f, 1.0f, 0.0f);
  glRotatef(rot_z, 0.0f, 0.0f, 1.0f);

  // Update Texture
  glBindTexture(GL_TEXTURE_2D, fire_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FIRE_WIDTH, FIRE_HEIGHT, 0, GL_BGRA,
               GL_UNSIGNED_BYTE, pixel_buffer);

  // Draw Cube
  glBegin(GL_QUADS);

  // Front Face
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);

  // Back Face
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);

  // Top Face
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);

  // Bottom Face
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);

  // Right face
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);

  // Left Face
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);

  glEnd();

  [[self openGLContext] flushBuffer];
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@property(strong) NSWindow *window;
@property(strong) FireGLView *view;
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
  [self.window setTitle:@"Fire Cube 3D"];
  [self.window center];
  [self.window setDelegate:self];

  // Create OpenGL View
  NSOpenGLPixelFormatAttribute attrs[] = {NSOpenGLPFADoubleBuffer,
                                          NSOpenGLPFAColorSize,
                                          24,
                                          NSOpenGLPFADepthSize,
                                          24,
                                          0};
  NSOpenGLPixelFormat *pf =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  self.view = [[FireGLView alloc] initWithFrame:frame pixelFormat:pf];
  [self.window setContentView:self.view];
  [self.window makeKeyAndOrderFront:nil];

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
  rot_x += 0.5f;
  rot_y += 0.8f;
  rot_z += 0.2f;
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
