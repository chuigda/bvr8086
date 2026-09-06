#ifndef BVR_GRAPHICS_H
#define BVR_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 图形模式描述符 */
typedef struct {
    uint16_t modeId;      /**< 模式标识符 */
    uint16_t width;       /**< 宽度（像素） */
    uint16_t height;      /**< 高度（像素） */
    uint8_t colorDepth;   /**< 色深（位） */
    bool supported;       /**< 是否支持 */
} BvrGraphicsMode;

/** 帧缓冲，存储 RGB888 像素 */
typedef struct {
    BvrGraphicsMode mode;
    uint32_t framebuffer[0];
} BvrFramebuffer;

/**
 * 检测可用的图形模式
 * @param pMode 模式数组
 * @param nMode 数组容量
 * @param offset 起始偏移量
 * @return 找到的支持模式数量，失败返回负数；
 *         若返回的数量少于 nMode，则表示所有支持的模式已经遍历完毕
 */
int16_t BvrDetectGraphics(BvrGraphicsMode *pMode, uint16_t nMode,
                          uint16_t offset);

/**
 * 初始化图形系统并分配帧缓冲
 * @param pMode 要初始化的图形模式
 * @return 帧缓冲指针，失败返回 NULL；返回的帧缓冲使用 free() 释放
 */
BvrFramebuffer* BvrInitGraphics(BvrGraphicsMode const *pMode);

/**
 * 将帧缓冲呈现到屏幕
 * @param pFramebuffer 帧缓冲
 */
void BvrSwapBuffers(BvrFramebuffer const *pFramebuffer);

/**
 * 关闭图形系统
 */
void BvrCloseGraphics(void);

/**
 * 顶点着色器：将顶点变换到 NDC 空间 [-1.0f, 1.0f] 并输出像素数据
 * @param pUniform 统一变量
 * @param pVertex 输入顶点数据
 * @param pPosition 输出位置（NDC 齐次坐标）
 * @param pPixelData 输出像素数据
 * @return false 表示裁剪该图元
 */
typedef bool (*BvrVertexShader)(void const *pUniform, void const *pVertex,
                                float (*pPosition)[4], void *pPixelData);

/**
 * 插值器：使用重心坐标插值三角形的像素数据
 * @param pPixelData 三个顶点的像素数据
 * @param pBarycentric 重心坐标 [w0, w1, w2]
 * @param pInterpolatedData 输出插值结果
 */
typedef void (*BvrInterpolate)(void const* (*pPixelData)[3],
                               float const (*pBarycentric)[3],
                               void *pInterpolatedData);

/**
 * 片段着色器：计算最终像素颜色
 * @param pUniform 统一变量
 * @param pPosition 片段位置
 * @param pPixelData 插值后的像素数据

 * @param pColor 输出颜色 RGB [0.0f, 1.0f]
 * @return false 表示丢弃该片段
 */
typedef bool (*BvrFragmentShader)(void const *pUniform,
                                  float const (*pPosition)[4],
                                  void const* pPixelData,
                                  float (*pColor)[3]);

/** 背面剔除模式 */
typedef enum {
    BVR_CULL_NONE = 0,  /**< 不剔除 */
    BVR_CULL_CCW = 1,   /**< 剔除逆时针三角形 */
    BVR_CULL_CW = 2     /**< 剔除顺时针三角形 */
} BvrCullMode;

/** 图形管线配置 */
typedef struct {
    BvrVertexShader vertexShader;      /**< 顶点着色器 */
    BvrInterpolate interpolate;        /**< 插值器 */
    BvrFragmentShader fragmentShader;  /**< 片段着色器 */
    void *pUniform;                    /**< 统一变量 */

    uint16_t vertexStride;             /**< 顶点结构体大小（字节） */
    uint16_t pixelDataStride;          /**< 像素数据大小（字节） */
    BvrCullMode cullMode;              /**< 背面剔除模式 */
} BvrGraphicsPipeline;

/** 深度缓冲 */
typedef struct {
    BvrGraphicsMode mode;
    float depthBuffer[0];
} BvrDepthBuffer;

/**
 * 创建深度缓冲
 * @param pFramebuffer 对应的帧缓冲
 * @return 深度缓冲指针，失败返回 NULL；返回的深度缓冲使用 free() 释放
 */
BvrDepthBuffer *BvrCreateDepthBuffer(BvrFramebuffer const *pFramebuffer);

/**
 * 绘制 3D 图元（直接模式）
 * @param pFramebuffer 帧缓冲
 * @param pDepthBuffer 深度缓冲
 * @param pPipeline 图形管线
 * @param pVertices 顶点数组
 * @param nVertices 顶点数量
 */
void BvrDraw3D(BvrFramebuffer *pFramebuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, uint32_t nVertices);

/**
 * 绘制 3D 图元（索引模式）
 * @param pFramebuffer 帧缓冲
 * @param pDepthBuffer 深度缓冲
 * @param pPipeline 图形管线
 * @param pVertices 顶点数组
 * @param pIndices 索引数组
 * @param nVertices 顶点数量
 * @param nIndices 索引数量
 * @param pPixelDataStorage 用于保存顶点着色器结果的缓冲区；
 *        若不为 NULL 则大小应为 pPipeline->pixelDataStride * nVertices；
 *        若为 NULL 则顶点着色器结果将存储于 alloca 分配的栈空间中，
 *        可能产生重复计算，影响性能
 */
void BvrDraw3DIndirect(BvrFramebuffer *pFramebuffer,
                       BvrDepthBuffer *pDepthBuffer,
                       BvrGraphicsPipeline const *pPipeline,
                       void const *pVertices, uint16_t const *pIndices,
                       uint32_t nVertices, uint32_t nIndices,
                       void *pPixelDataStorage);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_GRAPHICS_H */
