#include "Graphics.h"

#include "GraphImp.h"

void BvrPutPixel(BvrFrameBuffer *pFrameBuffer, uint16_t x, uint16_t y,
                 BvrColor3f color) {
  BvrImpPutPixel(pFrameBuffer, x, y, color);
}
