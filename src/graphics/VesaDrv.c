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