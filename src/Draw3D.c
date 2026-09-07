#include "Graphics.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Common.h"
#include "GraphImp.h"

static void NormalisePositions(BvrVec4f *pPosition);
static void MinMaxXY(BvrVec4f const (*pPositions)[3], float *pMinX,
                     float *pMaxX, float *pMinY, float *pMaxY);
static void Barycentric(BvrVec4f const (*pPositions)[3],
                        BvrVec4f const *pPoint,
                        float (*pBarycentric)[3]);

BvrDepthBuffer*
BvrCreateDepthBuffer(BvrFrameBuffer const *pFrameBuffer) {
  uint32_t size = (uint32_t)pFrameBuffer->mode.width
                  * (uint32_t)pFrameBuffer->mode.height
                  * sizeof(float);
  BvrDepthBuffer *pDepthBuffer = malloc(sizeof(BvrDepthBuffer) + size);
  if (!pDepthBuffer) {
    return NULL;
  }
  memset(pDepthBuffer, 0, sizeof(BvrDepthBuffer) + size);
  pDepthBuffer->mode = pFrameBuffer->mode;
  return pDepthBuffer;
}

void BvrDraw3D(BvrFrameBuffer *pFrameBuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, uint32_t nVertices) {
}

static void NormalisePositions(BvrVec4f *pPosition) {
  float w = pPosition->w;
  if (w != 0.0f) {
    pPosition->x /= w;
    pPosition->y /= w;
    pPosition->z /= w;
    pPosition->w = 1.0f;
  }
}

static void MinMaxXY(BvrVec4f const (*pPositions)[3], float *pMinX,
                     float *pMaxX, float *pMinY, float *pMaxY) {
  float minX = (*pPositions)[0].x,
        minY = (*pPositions)[0].y,
        maxX = (*pPositions)[0].x,
        maxY = (*pPositions)[0].y;
  for (uint32_t i = 1; i < 3; i += 1) {
    float x = (*pPositions)[i].x;
    float y = (*pPositions)[i].y;
    if (x < minX) minX = x;
    if (x > maxX) maxX = x;
    if (y < minY) minY = y;
    if (y > maxY) maxY = y;
  }

  *pMinX = minX;
  *pMaxX = maxX;
  *pMinY = minY;
  *pMaxY = maxY;
}

static void Barycentric(BvrVec4f const (*pPositions)[3],
                        BvrVec4f const *pPixel,
                        float (*barycentric)[3]) {
  BvrVec4f v1 = (*pPositions)[0];
  BvrVec4f v2 = (*pPositions)[1];
  BvrVec4f v3 = (*pPositions)[2];

  float denom = (v2.y - v3.y) * (v1.x - v3.x)
                + (v3.x - v2.x) * (v1.y - v3.y);
  (*barycentric)[0] = ((v2.y - v3.y) * (pPixel->x - v3.x)
                       + (v3.x - v2.x) * (pPixel->y - v3.y)) / denom;
  (*barycentric)[1] = ((v3.y - v1.y) * (pPixel->x - v3.x)
                       + (v1.x - v3.x) * (pPixel->y - v3.y)) / denom;
  (*barycentric)[2] = 1.0f - (*barycentric)[0] - (*barycentric)[1];
}
