#include "Graphics.h"

#include <assert.h>
#include <stdlib.h>

BvrFrameBuffer *BvrCreateFrameBuffer(uint16_t width, uint16_t height,
                                     uint8_t colorDepth) {
  size_t bytesPerPixel;
  switch (colorDepth) {
    case 8:  bytesPerPixel = 1; break;
    case 15:
    case 16: bytesPerPixel = 2; break;
    case 24: bytesPerPixel = 3; break;
    default: assert(false && "Unsupported color depth"); break;
  }

  size_t size = sizeof(BvrFrameBuffer) + width * height * bytesPerPixel;
  BvrFrameBuffer *pFrameBuffer = (BvrFrameBuffer *)malloc(size);
  if (!pFrameBuffer) {
    return NULL;
  }
  pFrameBuffer->width = width;
  pFrameBuffer->height = height;
  pFrameBuffer->colorDepth = colorDepth;
  return pFrameBuffer;
}

BvrDepthBuffer *BvrCreateDepthBuffer(uint16_t width, uint16_t height) {
  size_t size = sizeof(BvrDepthBuffer) + width * height * sizeof(float);
  BvrDepthBuffer *pDepthBuffer = (BvrDepthBuffer *)malloc(size);
  if (!pDepthBuffer) {
    return NULL;
  }
  pDepthBuffer->width = width;
  pDepthBuffer->height = height;
  return pDepthBuffer;
}
