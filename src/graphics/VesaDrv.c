#include "Graphics.h"

#include <stdlib.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>

typedef struct {
  uint8_t VESASignature[4];
  uint16_t VESAVersion;
  uint32_t OEMStringPtr;
  uint8_t capabilities[4];
  uint32_t videoModePtr;
  uint16_t totalMemory;
  uint16_t OEMSoftwareRev;
  uint32_t OEMVendorNamePtr;
  uint32_t OEMProductNamePtr;
  uint32_t OEMProductRevPtr;
  uint8_t reserved[222];
  uint8_t OEMData[256];
} __attribute__((packed)) VesaInfo;

typedef struct {
  uint16_t modeAttributes;
  uint8_t winAAttributes;
  uint8_t winBAttributes;
  uint16_t winGranularity;
  uint16_t winSize;
  uint16_t winASegment;
  uint16_t winBSegment;
  uint32_t winFuncPtr;
  uint16_t bytesPerScanLine;
  uint16_t xResolution;
  uint16_t yResolution;
  uint8_t xCharSize;
  uint8_t yCharSize;
  uint8_t numberOfPlanes;
  uint8_t bitsPerPixel;
  uint8_t numberOfBanks;
  uint8_t memoryModel;
  uint8_t bankSize;
  uint8_t numberOfImagePages;
  uint8_t reservedPage;
  uint8_t redMaskSize;
  uint8_t redMaskPos;
  uint8_t greenMaskSize;
  uint8_t greenMaskPos;
  uint8_t blueMaskSize;
  uint8_t blueMaskPos;
  uint8_t reservedMaskSize;
  uint8_t reservedMaskPos;
  uint8_t directColorModeInfo;
  uint32_t physBasePtr;
  uint32_t offScreenMemOffset;
  uint16_t offScreenMemSize;
  uint8_t reserved[206];
} __attribute__((packed)) VesaModeInfo;

static VesaModeInfo s_CurrentMode;
static bool s_GraphicsActive;
static uint16_t s_WriteWindow;
static unsigned long s_WindowAddress;
static bool s_UseLFB;
static uint8_t *s_LFBAddress;

static int GetVesaInfo(VesaInfo *pInfo) {
  __dpmi_regs r;
  long dosbuf = __tb & 0xFFFFF;

  for (int i = 0; i < sizeof(VesaInfo); i++) {
    _farpokeb(_dos_ds, dosbuf + i, 0);
  }

  dosmemput("VBE2", 4, dosbuf);

  r.x.ax = 0x4F00;
  r.x.di = dosbuf & 0xF;
  r.x.es = (dosbuf >> 4) & 0xFFFF;
  __dpmi_int(0x10, &r);

  if (r.h.ah != 0) {
    return -1;
  }

  dosmemget(dosbuf, sizeof(VesaInfo), pInfo);

  if (strncmp((char*)pInfo->VESASignature, "VESA", 4) != 0) {
    return -1;
  }

  return 0;
}

static int GetModeInfo(uint16_t mode, VesaModeInfo *pInfo) {
  __dpmi_regs r;
  long dosbuf = __tb & 0xFFFFF;

  for (int i = 0; i < sizeof(VesaModeInfo); i++) {
    _farpokeb(_dos_ds, dosbuf + i, 0);
  }

  r.x.ax = 0x4F01;
  r.x.di = dosbuf & 0xF;
  r.x.es = (dosbuf >> 4) & 0xFFFF;
  r.x.cx = mode;
  __dpmi_int(0x10, &r);

  if (r.h.ah != 0) {
    return -1;
  }

  dosmemget(dosbuf, sizeof(VesaModeInfo), pInfo);
  return 0;
}

static void SetVesaBank(uint16_t bankNumber) {
  __dpmi_regs r;
  r.x.ax = 0x4F05;
  r.x.bx = 0;
  r.x.dx = bankNumber;
  __dpmi_int(0x10, &r);
}

int16_t BvrDetectGraphics(BvrGraphicsMode *pMode, uint16_t nMode,
                          uint16_t offset) {
  VesaInfo vesaInfo;

  if (GetVesaInfo(&vesaInfo) != 0) {
    return -1;
  }

  unsigned long modePtr = ((vesaInfo.videoModePtr & 0xFFFF0000) >> 12) +
                          (vesaInfo.videoModePtr & 0xFFFF);

  uint16_t modeList[256];
  int modeCount = 0;

  while (_farpeekw(_dos_ds, modePtr) != 0xFFFF && modeCount < 256) {
    modeList[modeCount] = _farpeekw(_dos_ds, modePtr);
    modeCount++;
    modePtr += 2;
  }

  int foundCount = 0;
  int skipCount = 0;

  for (int i = 0; i < modeCount && foundCount < nMode; i++) {
    VesaModeInfo modeInfo;

    if (GetModeInfo(modeList[i], &modeInfo) != 0) {
      continue;
    }

    if ((modeInfo.modeAttributes & 0x19) != 0x19) {
      continue;
    }

    if (modeInfo.numberOfPlanes != 1) {
      continue;
    }

    if (modeInfo.memoryModel != 4 && modeInfo.memoryModel != 6) {
      continue;
    }

    if (modeInfo.bitsPerPixel != 8 && modeInfo.bitsPerPixel != 15 &&
        modeInfo.bitsPerPixel != 16 && modeInfo.bitsPerPixel != 24) {
      continue;
    }

    if (skipCount < offset) {
      skipCount++;
      continue;
    }

    pMode[foundCount].modeId = modeList[i];
    pMode[foundCount].width = modeInfo.xResolution;
    pMode[foundCount].height = modeInfo.yResolution;
    pMode[foundCount].colorDepth = modeInfo.bitsPerPixel;
    pMode[foundCount].supported = true;
    foundCount++;
  }

  return foundCount;
}

void BvrInitGraphics(BvrGraphicsMode const *pMode) {
  __dpmi_regs r;

  if (GetModeInfo(pMode->modeId, &s_CurrentMode) != 0) {
    return;
  }

  s_UseLFB = false;
  s_LFBAddress = NULL;

  if ((s_CurrentMode.modeAttributes & 0x80)
      && s_CurrentMode.physBasePtr != 0) {
    __dpmi_meminfo memInfo;
    memInfo.address = s_CurrentMode.physBasePtr;
    memInfo.size = (unsigned long)pMode->width * pMode->height *
                   ((pMode->colorDepth + 7) / 8);

    if (__dpmi_physical_address_mapping(&memInfo) == 0) {
      __dpmi_meminfo linInfo;
      linInfo.address = memInfo.address;
      linInfo.size = memInfo.size;

      if (__dpmi_allocate_linear_memory(&linInfo, 0) == 0) {
        s_LFBAddress = (uint8_t*)linInfo.address;
        s_UseLFB = true;
      }
    }
  }

  uint16_t modeNumber = pMode->modeId;
  if (s_UseLFB) {
    modeNumber |= 0x4000;
  }

  r.x.ax = 0x4F02;
  r.x.bx = modeNumber;
  __dpmi_int(0x10, &r);

  if (r.h.ah == 0) {
    s_GraphicsActive = true;
    s_WriteWindow = 0xFFFF;
    s_WindowAddress = (s_CurrentMode.winASegment != 0) ?
                      (s_CurrentMode.winASegment * 16) : 0xA0000;
  }
}

void BvrCloseGraphics(void) {
  if (!s_GraphicsActive) {
    return;
  }

  if (s_UseLFB && s_LFBAddress != NULL) {
    __dpmi_meminfo linInfo;
    linInfo.address = (unsigned long)s_LFBAddress;
    linInfo.size = 0;
    __dpmi_free_physical_address_mapping(&linInfo);
    s_LFBAddress = NULL;
  }

  __dpmi_regs r;
  r.x.ax = 0x0003;
  __dpmi_int(0x10, &r);

  s_GraphicsActive = false;
  s_UseLFB = false;
}

void BvrSwapBuffers(BvrFrameBuffer const *pFrameBuffer) {
  if (!s_GraphicsActive) {
    return;
  }

  uint32_t bytesPerPixel = (pFrameBuffer->colorDepth + 7) / 8;
  uint32_t screenSize = (uint32_t)pFrameBuffer->width * pFrameBuffer->height *
                        bytesPerPixel;

  if (s_UseLFB && s_LFBAddress != NULL) {
    memcpy(s_LFBAddress, pFrameBuffer->framebuffer, screenSize);
  } else {
    uint32_t bankSize = s_CurrentMode.winSize * 1024;
    uint32_t bankGranularity = s_CurrentMode.winGranularity * 1024;
    uint16_t bankNumber = 0;
    uint32_t bytesRemaining = screenSize;
    const uint8_t *srcPtr = pFrameBuffer->framebuffer;

    while (bytesRemaining > 0) {
      SetVesaBank(bankNumber);

      uint32_t copySize = (bytesRemaining > bankSize) ? bankSize : bytesRemaining;

      dosmemput(srcPtr, copySize, s_WindowAddress);

      bytesRemaining -= copySize;
      srcPtr += copySize;
      bankNumber += bankSize / bankGranularity;
    }
  }
}