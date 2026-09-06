#include "Graphics.h"
#include <stdio.h>
#include <conio.h>

int main(void) {
    BvrGraphicsMode modes[64];
    int16_t count = BvrDetectGraphics(modes, 64, 0);
    
    if (count < 0) {
        printf("Failed to detect VESA graphics\n");
        return 1;
    }
    
    printf("Found %d graphics modes:\n", count);
    for (int i = 0; i < count && i < 20; i++) {
        printf("%2d: %4dx%4d %2d-bit (mode 0x%04X)\n", 
               i, modes[i].width, modes[i].height, 
               modes[i].colorDepth, modes[i].modeId);
    }
    
    printf("\nTesting 640x480 8-bit mode...\n");
    BvrGraphicsMode testMode = {0, 640, 480, 8};
    BvrFramebuffer *fb = BvrInitGraphics(&testMode);
    
    if (fb == NULL) {
        printf("Failed to initialize graphics mode\n");
        return 1;
    }
    
    printf("Graphics initialized successfully!\n");
    printf("Drawing test pattern...\n");
    
    uint8_t *pixels = (uint8_t*)fb->framebuffer;
    for (int y = 0; y < 480; y++) {
        for (int x = 0; x < 640; x++) {
            uint8_t color = (x / 8 + y / 8) & 0xFF;
            pixels[y * 640 + x] = color;
        }
    }
    
    BvrSwapBuffers(fb);
    getch();
    
    BvrCloseGraphics();
    printf("Graphics closed\n");
    
    return 0;
}
