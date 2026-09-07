#ifndef BVR_GRAPHICS_H
#define BVR_GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

#include "LinAlg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 图形模式描述符 */
typedef struct stBvrGraphicsMode {
  uint16_t modeId;      /**< 模式标识符 */
  uint16_t width;       /**< 宽度（像素） */
  uint16_t height;      /**< 高度（像素） */
  uint8_t colorDepth;   /**< 色深（位） */
  bool supported;       /**< 是否支持 */
} BvrGraphicsMode;

/** 帧缓冲 */
typedef struct stBvrFrameBuffer {
  uint16_t width;
  uint16_t height;
  uint8_t colorDepth;
  uint8_t : 8;
  uint8_t framebuffer[0];
} BvrFrameBuffer;

/** 深度缓冲 */
typedef struct stBvrDepthBuffer {
  uint16_t width;
  uint16_t height;

  float depthBuffer[0];
} BvrDepthBuffer;

/** 背面剔除模式 */
typedef enum eBvrCullMode {
  BVR_CULL_NONE = 0,  /**< 不剔除 */
  BVR_CULL_CCW = 1,   /**< 剔除逆时针三角形 */
  BVR_CULL_CW = 2     /**< 剔除顺时针三角形 */
} BvrCullMode;

/**
 * 顶点着色器：将顶点变换到 NDC 空间 [-1.0f, 1.0f] 并输出像素数据
 * @param pUniform 统一变量
 * @param pVertex 输入顶点数据
 * @param pPosition 输出位置（NDC 齐次坐标）
 * @param pPixelData 输出像素数据
 * @return false 表示裁剪该图元Bv
 */
typedef bool (*BvrVertexShader)(void const *pUniform,
                                void const *pVertex,
                                BvrVec4f const *pPosition,
                                void *pPixelData);

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
                                  BvrVec4f const *pPosition,
                                  void const* pPixelData,
                                  BvrColor3f *pColor);

/** 图形管线配置 */
typedef struct stBvrGraphicsPipeline {
  BvrVertexShader vertexShader;      /**< 顶点着色器 */
  BvrInterpolate interpolate;        /**< 插值器，为 NULL 则扁平插值 */
  BvrFragmentShader fragmentShader;  /**< 片段着色器 */
  void *pUniform;                    /**< 统一变量 */

  uint16_t vertexStride;             /**< 顶点结构体大小（字节） */
  uint16_t pixelDataStride;          /**< 像素数据大小（字节） */
  BvrCullMode cullMode;              /**< 背面剔除模式 */
} BvrGraphicsPipeline;

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
 * 初始化图形系统
 * @param pMode 要初始化的图形模式
 */
void BvrInitGraphics(BvrGraphicsMode const *pMode);

/**
 * 关闭图形系统
 */
void BvrCloseGraphics(void);

/**
 * 将帧缓冲呈现到屏幕
 * @param pFrameBuffer 帧缓冲
 */
void BvrSwapBuffers(BvrFrameBuffer const *pFrameBuffer);

/**
 * 创建帧缓冲
 * @param width 帧缓冲宽度
 * @param height 帧缓冲高度
 * @param colorDepth 帧缓冲色深
 * @return 帧缓冲指针，失败返回 NULL；返回的帧缓冲使用 free() 释放
 */
BvrFrameBuffer* BvrCreateFrameBuffer(uint16_t width, uint16_t height,
                                     uint8_t colorDepth);

/**
 * 创建深度缓冲
 * @param width 深度缓冲宽度
 * @param height 深度缓冲高度
 * @return 深度缓冲指针，失败返回 NULL；返回的深度缓冲使用 free() 释放
 */
BvrDepthBuffer *BvrCreateDepthBuffer(uint16_t width, uint16_t height);

/**
 * 在帧缓冲中绘制像素
 * @param pFrameBuffer 帧缓冲
 * @param x 像素横坐标
 * @param y 像素纵坐标
 * @param color 像素颜色 RGB [0.0f, 1.0f]
 */
void BvrPutPixel(BvrFrameBuffer *pFrameBuffer, uint16_t x, uint16_t y,
                 BvrColor3f color);

typedef struct stBvrBitmap BvrBitmap;

/**
 * 在帧缓冲中绘制线段
 * @param pFrameBuffer 帧缓冲
 * @param x0 起点横坐标
 * @param y0 起点纵坐标
 * @param x1 终点横坐标
 * @param y1 终点纵坐标
 * @param color 线段颜色 RGB [0.0f, 1.0f]
 */
void BvrLine(BvrFrameBuffer *pFrameBuffer, uint16_t x0, uint16_t y0,
             uint16_t x1, uint16_t y1, BvrColor3f color);

/**
 * 在帧缓冲中绘制水平线段
 * @param pFrameBuffer 帧缓冲
 * @param x0 起点横坐标
 * @param x1 终点横坐标
 * @param y 纵坐标
 * @param length 线段长度
 * @param color 线段颜色 RGB [0.0f, 1.0f]
 */
void BvrLineX(BvrFrameBuffer *pFrameBuffer, uint16_t x0, uint16_t x1,
              uint16_t y, BvrColor3f color);

/**
 * 在帧缓冲中绘制垂直线段
 * @param pFrameBuffer 帧缓冲
 * @param x 横坐标
 * @param y0 起点纵坐标
 * @param y1 终点纵坐标
 * @param color 线段颜色 RGB [0.0f, 1.0f]
 */
void BvrLineY(BvrFrameBuffer *pFrameBuffer, uint16_t x, uint16_t y0,
              uint16_t y1, BvrColor3f color);

/**
 * 在帧缓冲中绘制位图
 * @param pFrameBuffer 帧缓冲
 * @param pBitmap 位图
 * @param x 位图左上角横坐标
 * @param y 位图左上角纵坐标
 */
void BvrPutBitmap(BvrFrameBuffer *pFrameBuffer, BvrBitmap const *pBitmap,
                  uint16_t x, uint16_t y);

/**
 * 在帧缓冲中绘制位图-带旋转
 * @param pFrameBuffer 帧缓冲
 * @param pBitmap 位图
 * @param x 位图中心横坐标
 * @param y 位图中心纵坐标
 * @param angle 旋转角度（弧度）
 */
void BvrPutBitmapRotated(BvrFrameBuffer *pFrameBuffer,
                         BvrBitmap const *pBitmap, uint16_t x,
                         uint16_t y, float angle);

/**
 * 绘制 3D 图元（直接模式）
 * @param pFrameBuffer 帧缓冲
 * @param pDepthBuffer 深度缓冲，可为 NULL
 * @param pPipeline 图形管线
 * @param pVertices 顶点数组
 * @param nVertices 顶点数量
 */
void BvrDraw3D(BvrFrameBuffer *pFrameBuffer,
               BvrDepthBuffer *pDepthBuffer,
               BvrGraphicsPipeline const *pPipeline,
               void const *pVertices, uint32_t nVertices);

/**
 * 绘制 3D 图元（索引模式）
 * @param pFrameBuffer 帧缓冲
 * @param pDepthBuffer 深度缓冲，可为 NULL
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
void BvrDraw3DIndexed(BvrFrameBuffer *pFrameBuffer,
                      BvrDepthBuffer *pDepthBuffer,
                      BvrGraphicsPipeline const *pPipeline,
                      void const *pVertices, uint16_t const *pIndices,
                      uint32_t nVertices, uint32_t nIndices,
                      void *pPixelDataStorage);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_GRAPHICS_H */
