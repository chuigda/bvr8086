#ifndef BVR_GRAPHICS_H
#define BVR_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t nWidth;
    uint16_t nHeight;
    uint8_t bColorDepth;
    uint8_t bModeID;
} BvrGraphicsMode;

typedef struct {
    BvrGraphicsMode mode;
    uint16_t buffer[0];
} BvrFramebuffer;

int BvrDetectGraphics(BvrGraphicsMode *pMode, uint16_t nMode);
BvrFramebuffer* BvrInitGraphics(BvrGraphicsMode const *pMode);
void BvrSwapBuffers(BvrFramebuffer const *pFramebuffer);
void BvrCloseGraphics(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_GRAPHICS_H */
