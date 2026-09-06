#include "Graphics.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

BvrGraphicsPipeline BvrCreatePipeline(BvrVertexShader vertexShader,
                                      BvrInterpolate interpolate,
                                      BvrFragmentShader fragmentShader,
                                      void *pUniform,
                                      uint16_t vertexStride,
                                      uint16_t pixelDataStride,
                                      BvrCullMode cullMode) {
    BvrGraphicsPipeline pipeline;
    pipeline.vertexShader = vertexShader;
    pipeline.interpolate = interpolate;
    pipeline.fragmentShader = fragmentShader;
    pipeline.pUniform = pUniform;
    pipeline.vertexStride = vertexStride;
    pipeline.pixelDataStride = pixelDataStride;
    pipeline.cullMode = cullMode;
    return pipeline;
}

BvrDepthBuffer *BvrCreateDepthBuffer(BvrFramebuffer const *pFramebuffer) {
    if (pFramebuffer == NULL) return NULL;
    
    uint32_t numPixels = pFramebuffer->mode.width * pFramebuffer->mode.height;
    uint32_t bufferSize = sizeof(BvrDepthBuffer) + numPixels * sizeof(float);
    
    BvrDepthBuffer *depthBuffer = (BvrDepthBuffer*)malloc(bufferSize);
    if (depthBuffer == NULL) return NULL;
    
    depthBuffer->mode = pFramebuffer->mode;
    
    for (uint32_t i = 0; i < numPixels; i++) {
        depthBuffer->depthBuffer[i] = 1.0f;
    }
    
    return depthBuffer;
}

static inline uint32_t PackColor(float const color[3], uint8_t bpp) {
    uint8_t r = (uint8_t)(color[0] * 255.0f);
    uint8_t g = (uint8_t)(color[1] * 255.0f);
    uint8_t b = (uint8_t)(color[2] * 255.0f);
    
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    
    switch (bpp) {
        case 8:
            return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
        case 16:
            return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        case 24:
        case 32:
            return (r << 16) | (g << 8) | b;
        default:
            return 0;
    }
}

static void RasterizeTriangle(BvrFramebuffer *pFramebuffer,
                              BvrDepthBuffer *pDepthBuffer,
                              BvrGraphicsPipeline const *pPipeline,
                              float const v0[4], float const v1[4], float const v2[4],
                              void const *pd0, void const *pd1, void const *pd2) {
    float x0 = v0[0], y0 = v0[1], z0 = v0[2];
    float x1 = v1[0], y1 = v1[1], z1 = v1[2];
    float x2 = v2[0], y2 = v2[1], z2 = v2[2];
    
    int16_t width = pFramebuffer->mode.width;
    int16_t height = pFramebuffer->mode.height;
    uint8_t bpp = pFramebuffer->mode.colorDepth;
    
    float ndcX0 = (x0 + 1.0f) * 0.5f * width;
    float ndcY0 = (1.0f - y0) * 0.5f * height;
    float ndcX1 = (x1 + 1.0f) * 0.5f * width;
    float ndcY1 = (1.0f - y1) * 0.5f * height;
    float ndcX2 = (x2 + 1.0f) * 0.5f * width;
    float ndcY2 = (1.0f - y2) * 0.5f * height;
    
    int minX = (int)fminf(fminf(ndcX0, ndcX1), ndcX2);
    int maxX = (int)ceilf(fmaxf(fmaxf(ndcX0, ndcX1), ndcX2));
    int minY = (int)fminf(fminf(ndcY0, ndcY1), ndcY2);
    int maxY = (int)ceilf(fmaxf(fmaxf(ndcY0, ndcY1), ndcY2));
    
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= width) maxX = width - 1;
    if (maxY >= height) maxY = height - 1;
    
    float area = (ndcX1 - ndcX0) * (ndcY2 - ndcY0) - (ndcX2 - ndcX0) * (ndcY1 - ndcY0);
    if (fabsf(area) < 0.001f) return;
    
    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float px = x + 0.5f;
            float py = y + 0.5f;
            
            float w0 = ((ndcX1 - ndcX0) * (py - ndcY0) - (ndcY1 - ndcY0) * (px - ndcX0)) / area;
            float w1 = ((ndcX2 - ndcX1) * (py - ndcY1) - (ndcY2 - ndcY1) * (px - ndcX1)) / area;
            float w2 = ((ndcX0 - ndcX2) * (py - ndcY2) - (ndcY0 - ndcY2) * (px - ndcX2)) / area;
            
            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f) continue;
            
            float barycentric[3] = {w2, w0, w1};
            float depth = z0 * barycentric[0] + z1 * barycentric[1] + z2 * barycentric[2];
            
            uint32_t pixelIndex = y * width + x;
            
            if (depth <= pDepthBuffer->depthBuffer[pixelIndex]) {
                pDepthBuffer->depthBuffer[pixelIndex] = depth;
                
                uint8_t interpolatedData[256];
                void const *pixelDataArray[3] = {pd0, pd1, pd2};
                pPipeline->interpolate((void const **)pixelDataArray, &barycentric, interpolatedData);
                
                float fragPos[4] = {x0 * barycentric[0] + x1 * barycentric[1] + x2 * barycentric[2],
                                    y0 * barycentric[0] + y1 * barycentric[1] + y2 * barycentric[2],
                                    depth, 1.0f};
                
                float color[3];
                if (pPipeline->fragmentShader(pPipeline->pUniform, &fragPos, 
                                             interpolatedData, &color)) {
                    uint32_t packedColor = PackColor(color, bpp);
                    
                    if (bpp == 8) {
                        ((uint8_t*)pFramebuffer->framebuffer)[pixelIndex] = (uint8_t)packedColor;
                    } else if (bpp == 16) {
                        ((uint16_t*)pFramebuffer->framebuffer)[pixelIndex] = (uint16_t)packedColor;
                    } else if (bpp == 24) {
                        uint8_t *pixel = &((uint8_t*)pFramebuffer->framebuffer)[pixelIndex * 3];
                        pixel[0] = packedColor & 0xFF;
                        pixel[1] = (packedColor >> 8) & 0xFF;
                        pixel[2] = (packedColor >> 16) & 0xFF;
                    } else if (bpp == 32) {
                        ((uint32_t*)pFramebuffer->framebuffer)[pixelIndex] = packedColor;
                    }
                }
            }
        }
    }
}

void BvrDraw3D(BvrFramebuffer *pFramebuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, void *pVertexDataStorage,
               uint32_t nVertices) {
    if (pFramebuffer == NULL || pDepthBuffer == NULL || 
        pPipeline == NULL || pVertices == NULL || nVertices < 3) {
        return;
    }
    
    uint8_t const *vertexPtr = (uint8_t const*)pVertices;
    uint8_t *pixelDataPtr = (uint8_t*)pVertexDataStorage;
    
    for (uint32_t i = 0; i + 2 < nVertices; i += 3) {
        float positions[3][4];
        void *pixelData[3];
        
        for (int j = 0; j < 3; j++) {
            void const *vertex = vertexPtr + (i + j) * pPipeline->vertexStride;
            void *pd = pixelDataPtr + (i + j) * pPipeline->pixelDataStride;
            
            if (!pPipeline->vertexShader(pPipeline->pUniform, vertex, &positions[j], pd)) {
                goto skip_triangle;
            }
            
            pixelData[j] = pd;
        }
        
        if (pPipeline->cullMode != BVR_CULL_NONE) {
            float dx1 = positions[1][0] - positions[0][0];
            float dy1 = positions[1][1] - positions[0][1];
            float dx2 = positions[2][0] - positions[0][0];
            float dy2 = positions[2][1] - positions[0][1];
            float cross = dx1 * dy2 - dy1 * dx2;
            
            if ((pPipeline->cullMode == BVR_CULL_CCW && cross < 0.0f) ||
                (pPipeline->cullMode == BVR_CULL_CW && cross > 0.0f)) {
                continue;
            }
        }
        
        RasterizeTriangle(pFramebuffer, pDepthBuffer, pPipeline,
                         positions[0], positions[1], positions[2],
                         pixelData[0], pixelData[1], pixelData[2]);
        
    skip_triangle:
        continue;
    }
}

void BvrDraw3DIndirect(BvrFramebuffer *pFramebuffer,
                       BvrDepthBuffer *pDepthBuffer,
                       BvrGraphicsPipeline const *pPipeline,
                       void const *pVertices, uint16_t const *pIndices,
                       void *pVertexDataStorage, uint32_t nIndices) {
    if (pFramebuffer == NULL || pDepthBuffer == NULL || 
        pPipeline == NULL || pVertices == NULL || 
        pIndices == NULL || nIndices < 3) {
        return;
    }
    
    uint8_t const *vertexPtr = (uint8_t const*)pVertices;
    uint8_t *pixelDataPtr = (uint8_t*)pVertexDataStorage;
    
    for (uint32_t i = 0; i + 2 < nIndices; i += 3) {
        float positions[3][4];
        void *pixelData[3];
        
        for (int j = 0; j < 3; j++) {
            uint16_t idx = pIndices[i + j];
            void const *vertex = vertexPtr + idx * pPipeline->vertexStride;
            void *pd = pixelDataPtr + idx * pPipeline->pixelDataStride;
            
            if (!pPipeline->vertexShader(pPipeline->pUniform, vertex, &positions[j], pd)) {
                goto skip_triangle;
            }
            
            pixelData[j] = pd;
        }
        
        if (pPipeline->cullMode != BVR_CULL_NONE) {
            float dx1 = positions[1][0] - positions[0][0];
            float dy1 = positions[1][1] - positions[0][1];
            float dx2 = positions[2][0] - positions[0][0];
            float dy2 = positions[2][1] - positions[0][1];
            float cross = dx1 * dy2 - dy1 * dx2;
            
            if ((pPipeline->cullMode == BVR_CULL_CCW && cross < 0.0f) ||
                (pPipeline->cullMode == BVR_CULL_CW && cross > 0.0f)) {
                continue;
            }
        }
        
        RasterizeTriangle(pFramebuffer, pDepthBuffer, pPipeline,
                         positions[0], positions[1], positions[2],
                         pixelData[0], pixelData[1], pixelData[2]);
        
    skip_triangle:
        continue;
    }
}

