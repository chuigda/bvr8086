#include "Graphics.h"
#include "PtrMath.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

static void NormalisePositions(float (*pPosition)[4]);
static void MinMaxXY(float const (*pPositions)[12], float *pMinX,
                     float *pMaxX, float *pMinY, float *pMaxY);
static void Barycentric(float const (*pPositions)[12],
                        float const (*pPoint)[4],
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
  memset(pDepthBuffer + 1, 0, size);
  pDepthBuffer->mode = pFrameBuffer->mode;
  return pDepthBuffer;
}

void BvrDraw3D(BvrFrameBuffer *pFrameBuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, uint32_t nVertices) {
    float position[12];
    void *pPixelDataArray = alloca(pPipeline->pixelDataStride * 3);
    void *pPixelDataInterp = alloca(pPipeline->pixelDataStride);

    for (uint32_t i = 0; i < nVertices; i += 3) {
      bool emitGlyph = true;

      for (uint32_t j = 0; j < 3; j += 1) {
        uint32_t vertexOffset = (i + j) * pPipeline->vertexStride;
        uint32_t positionOffset = j * 4;
        uint32_t pixelDataOffset = j * pPipeline->pixelDataStride;

        void const *pVertexData = PTR_ADD(pVertices, vertexOffset);
        float (*pPosition)[4] = (float (*)[4])(position + positionOffset);
        void *pPixelData = PTR_ADD(pPixelDataArray, pixelDataOffset);

        emitGlyph &= pPipeline->vertexShader(pPipeline->pUniform,
                                             pVertexData,
                                             pPosition,
                                             pPixelData);
        if (!emitGlyph) {
          break;
        }

        NormalisePositions(pPosition);
      }

      if (!emitGlyph) {
        continue;
      }

      float minX, maxX, minY, maxY;
      MinMaxXY(&position, &minX, &maxX, &minY, &maxY);

      uint16_t minXi = minX * pFrameBuffer->mode.width;
      uint16_t maxXi = maxX * pFrameBuffer->mode.width;
      uint16_t minYi = minY * pFrameBuffer->mode.height;
      uint16_t maxYi = maxY * pFrameBuffer->mode.height;
    }
}

static void NormalisePositions(float (*pPosition)[4]) {
  float w = (*pPosition)[3];
  if (w != 0.0f) {
    (*pPosition)[0] /= w;
    (*pPosition)[1] /= w;
    (*pPosition)[2] /= w;
    (*pPosition)[3] = 1.0f;
  }
}

static void MinMaxXY(float const (*pPositions)[12], float *pMinX,
                     float *pMaxX, float *pMinY, float *pMaxY) {
  float minX = (*pPositions)[0],
        minY = (*pPositions)[1],
        maxX = (*pPositions)[0],
        maxY = (*pPositions)[1];
  for (uint32_t i = 1; i < 3; i += 1) {
    float x = (*pPositions)[i * 4 + 0];
    float y = (*pPositions)[i * 4 + 1];
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
