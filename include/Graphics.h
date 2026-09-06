#ifndef BVR_GRAPHICS_H
#define BVR_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t modeId;
    uint16_t width;
    uint16_t height;
    uint8_t colorDepth;
} BvrGraphicsMode;

typedef struct {
    BvrGraphicsMode mode;
    uint32_t framebuffer[0];
} BvrFramebuffer;

int16_t BvrDetectGraphics(BvrGraphicsMode *pMode, uint16_t nMode);
BvrFramebuffer* BvrInitGraphics(BvrGraphicsMode const *pMode);
void BvrSwapBuffers(BvrFramebuffer const *pFramebuffer);
void BvrCloseGraphics(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_GRAPHICS_H */
