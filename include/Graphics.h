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

int16_t BvrDetectGraphics(BvrGraphicsMode *pMode, uint16_t nMode,
                          uint16_t offset);
BvrFramebuffer* BvrInitGraphics(BvrGraphicsMode const *pMode);
void BvrSwapBuffers(BvrFramebuffer const *pFramebuffer);
void BvrCloseGraphics(void);

typedef bool (*BvrVertexShader)(void const *pUniform, void const *pVertex,
                                float (*pPosition)[4], void *pPixelData);
typedef void (*BvrInterpolate)(void const (*pPixelData)[3],
                               float const (*pBarycentric)[3],
                               void *pInterpolatedData);
typedef bool (*BvrFragmentShader)(void const *pUniform,
                                  float const (*pPosition)[4],
                                  void const* pPixelData,
                                  float (*pColor)[3]);

typedef struct {
    BvrVertexShader vertexShader;
    BvrInterpolate interpolate;
    BvrFragmentShader fragmentShader;
    void *pUniform;

    uint16_t vertexStride;
    uint16_t pixelDataStride;
    uint32_t pixelDataBuffer[0];
} BvrGraphicsPipeline;

BvrGraphicsPipeline BvrCreatePipeline(BvrVertexShader vertexShader,
                                      BvrInterpolate interpolate,
                                      BvrFragmentShader fragmentShader,
                                      void *pUniform,
                                      uint16_t vertexStride,
                                      uint16_t pixelDataStride);

typedef struct {
    BvrGraphicsMode mode;
    float depthBuffer[0];
} BvrDepthBuffer;

BvrDepthBuffer *BvrCreateDepthBuffer(BvrFramebuffer const *pFramebuffer);

void BvrDraw3D(BvrFramebuffer *pFramebuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, void *pVertexDataStorage,
               uint32_t nVertices);

void BvrDraw3DIndirect(BvrFramebuffer *pFramebuffer,
                       BvrDepthBuffer *pDepthBuffer,
                       BvrGraphicsPipeline const *pPipeline,
                       void const *pVertices, uint16_t const *pIndices,
                       void *pVertexDataStorage, uint32_t nIndices);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_GRAPHICS_H */
