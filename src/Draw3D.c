#include "Graphics.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "PtrMath.h"
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
  memset(pDepthBuffer + 1, 0, size);
  pDepthBuffer->mode = pFrameBuffer->mode;
  return pDepthBuffer;
}

void BvrDraw3D(BvrFrameBuffer *pFrameBuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, uint32_t nVertices) {
  BvrVec4f position[3];
  void *pPixelDataArray = alloca(pPipeline->pixelDataStride * 3);
  void *pPixelDataInterp = alloca(pPipeline->pixelDataStride);

  for (uint32_t i = 0; i < nVertices; i += 3) {
    bool emitGlyph = true;

    for (uint32_t j = 0; j < 3; j += 1) {
      uint32_t vertexOffset = (i + j) * pPipeline->vertexStride;
      uint32_t positionOffset = j * 4;
      uint32_t pixelDataOffset = j * pPipeline->pixelDataStride;

      void const *pVertexData = PTR_ADD(pVertices, vertexOffset);
      BvrVec4f *pPosition = &position[j];
      void *pPixelData = PTR_ADD(pPixelDataArray, pixelDataOffset);

      emitGlyph &= pPipeline->vertexShader(pPipeline->pUniform,
                                           pVertexData, pPosition,
                                           pPixelData);
      if (!emitGlyph) {
        break;
      }

      NormalisePositions(pPosition);
    }

    if (!emitGlyph) {
      continue;
    }

    BvrVec4f p1p2 = BvrVecSub4f(position[1], position[0]);
    BvrVec4f p2p3 = BvrVecSub4f(position[2], position[1]);
    BvrVec3f normal = BvrVecCross4f(p1p2, p2p3);
    if ((normal.z > 0.0f && pPipeline->cullMode == BVR_CULL_CCW)
        || (normal.z < 0.0f && pPipeline->cullMode == BVR_CULL_CW)) {
      continue;
    }

    float minX, maxX, minY, maxY;
    MinMaxXY(&position, &minX, &maxX, &minY, &maxY);
    uint16_t minXi = minX * pFrameBuffer->mode.width;
    uint16_t maxXi = maxX * pFrameBuffer->mode.width;
    uint16_t minYi = minY * pFrameBuffer->mode.height;
    uint16_t maxYi = maxY * pFrameBuffer->mode.height;

    for (uint16_t y = minYi; y <= maxYi; y += 1) {
      for (uint16_t x = minXi; x <= maxXi; x += 1) {
        float xf = (float)x / pFrameBuffer->mode.width;
        float yf = (float)y / pFrameBuffer->mode.height;

        BvrVec4f pixel = (BvrVec4f){ xf, yf, 0.0f, 1.0f };
        float barycentric[3];
        Barycentric(&position, &pixel, &barycentric);

        if (barycentric[0] < 0.0f
            || barycentric[1] < 0.0f
            || barycentric[2] < 0.0f) {
          continue;
        }

        void* pPixelData[3] = {
          pPixelDataArray,
          PTR_ADD(pPixelDataArray, pPipeline->pixelDataStride),
          PTR_ADD(pPixelDataArray, 2 * pPipeline->pixelDataStride)
        };

        pPipeline->interpolate((void const* (*)[3])&pPixelData,
                               &barycentric,
                               pPixelDataArray);

        BvrColor3f color;
        if (pPipeline->fragmentShader(pPipeline->pUniform, &pixel,
                                      pPixelDataArray, &color)) {
          BvrImpPutPixel(pFrameBuffer, x, y, color);
        }
      }
    }
  }
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
