#ifndef BVR_INTERNAL_GRAPHICS_IMP_H
#define BVR_INTERNAL_GRAPHICS_IMP_H

#include "Graphics.h"

#include <assert.h>

__attribute__((always_inline)) static inline
void BvrImpPutPixel(BvrFrameBuffer *pFrameBuffer, uint16_t x, uint16_t y,
                    BvrColor3f color) {
  if (x >= pFrameBuffer->mode.width || y >= pFrameBuffer->mode.height) {
    return;
  }

  uint32_t offset = y * pFrameBuffer->mode.width + x;

  switch (pFrameBuffer->mode.colorDepth) {
    case 8: {
      uint8_t r = (uint8_t)(color.r * 7.0f);
      uint8_t g = (uint8_t)(color.g * 7.0f);
      uint8_t b = (uint8_t)(color.b * 3.0f);
      uint8_t *fb8 = (uint8_t *)pFrameBuffer->framebuffer;
      fb8[offset] = (r << 5) | (g << 2) | b;
      break;
    }
    case 15: {
      uint8_t r = (uint8_t)(color.r * 31.0f);
      uint8_t g = (uint8_t)(color.g * 31.0f);
      uint8_t b = (uint8_t)(color.b * 31.0f);
      uint16_t *fb16 = (uint16_t *)pFrameBuffer->framebuffer;
      fb16[offset] = (r << 10) | (g << 5) | b;
      break;
    }
    case 16: {
      uint8_t r = (uint8_t)(color.r * 31.0f);
      uint8_t g = (uint8_t)(color.g * 63.0f);
      uint8_t b = (uint8_t)(color.b * 31.0f);
      uint16_t *fb16 = (uint16_t *)pFrameBuffer->framebuffer;
      fb16[offset] = (r << 11) | (g << 5) | b;
      break;
    }
    case 24: {
      uint8_t r = (uint8_t)(color.r * 255.0f);
      uint8_t g = (uint8_t)(color.g * 255.0f);
      uint8_t b = (uint8_t)(color.b * 255.0f);
      pFrameBuffer->framebuffer[offset] = (r << 16) | (g << 8) | b;
      break;
    }
    default: assert(false && "Unsupported color depth");
  }
}

#endif /* BVR_INTERNAL_GRAPHICS_IMP_H */
