#ifndef BVR_BITMAP_H
#define BVR_BITMAP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 位图结构体 */
typedef struct stBvrBitmap {
  uint16_t width;        /**< 宽度（像素） */
  uint16_t height;       /**< 高度（像素） */
  uint8_t colorDepth;    /**< 色深（位） */
  uint32_t pixelData[0]; /**< 像素数据，从上到下、从左到右存储 */
} BvrBitmap;

/**
 * 从 BMP 文件加载位图
 * @param filename BMP 文件路径
 * @return 位图指针，失败返回 NULL；返回的位图使用 free() 释放
 */
BvrBitmap *BvrLoadBitmap(const char *filename);

/**
 * 采样位图
 * @param pBitmap 位图指针
 * @param u 水平纹理坐标 [0, 1]
 * @param v 垂直纹理坐标 [0, 1]
 * @param linear 是否使用线性插值
 * @param pColor 输出颜色，长度为 3 的浮点数组（RGB）
 */
void BvrSampleBitmap(BvrBitmap const *pBitmap, float u, float v,
                     bool linear, float (*pColor)[3]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_BITMAP_H */
