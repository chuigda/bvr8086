#include "Graphics.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

int main(void) {
    BvrGraphicsMode modes[64];
    int16_t count;

    printf("Detecting VESA graphics modes...\n");
    count = BvrDetectGraphics(modes, 64, 0);

    if (count < 0) {
        printf("Error: VESA not available\n");
        return 1;
    }

    printf("Found %d graphics modes:\n", count);
    for (int i = 0; i < count; i++) {
        printf("  Mode 0x%04X: %dx%d @ %d-bit\n",
               modes[i].modeId, modes[i].width, modes[i].height,
               modes[i].colorDepth);
    }

    BvrGraphicsMode *targetMode = NULL;
    for (int i = 0; i < count; i++) {
        if (modes[i].width == 800 && modes[i].height == 600 &&
            modes[i].colorDepth == 16) {
            targetMode = &modes[i];
            break;
        }
    }

    if (!targetMode) {
        printf("\nError: 800x600x16 mode not found\n");
        return 1;
    }

    printf("\nUsing mode 0x%04X: 800x600x16\n", targetMode->modeId);
    printf("Press any key to start...\n");
    getch();

    BvrInitGraphics(targetMode);

    BvrFrameBuffer *fb = BvrCreateFrameBuffer(800, 600, 16);
    if (!fb) {
        BvrCloseGraphics();
        printf("Error: Failed to create framebuffer\n");
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        for (int y = 0; y < 600; y++) {
            for (int x = 0; x < 800; x++) {
                BvrColor3f color = {
                    (float)x / 800.0f,
                    (float)y / 600.0f,
                    (float)i / 100.0f
                };
                BvrPutPixel(fb, x, y, color);
            }
        }

        BvrSwapBuffers(fb);

        if (kbhit()) break;
    }

    getch();

    free(fb);
    BvrCloseGraphics();

    printf("Done!\n");
    return 0;
}
