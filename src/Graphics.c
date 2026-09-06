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
    uint8_t Capabilities[4];
    uint32_t VideoModePtr;
    uint16_t TotalMemory;
    uint16_t OemSoftwareRev;
    uint32_t OemVendorNamePtr;
    uint32_t OemProductNamePtr;
    uint32_t OemProductRevPtr;
    uint8_t Reserved[222];
    uint8_t OemData[256];
} __attribute__((packed)) VesaInfo;

typedef struct {
    uint16_t ModeAttributes;
    uint8_t WinAAttributes;
    uint8_t WinBAttributes;
    uint16_t WinGranularity;
    uint16_t WinSize;
    uint16_t WinASegment;
    uint16_t WinBSegment;
    uint32_t WinFuncPtr;
    uint16_t BytesPerScanLine;
    uint16_t XResolution;
    uint16_t YResolution;
    uint8_t XCharSize;
    uint8_t YCharSize;
    uint8_t NumberOfPlanes;
    uint8_t BitsPerPixel;
    uint8_t NumberOfBanks;
    uint8_t MemoryModel;
    uint8_t BankSize;
    uint8_t NumberOfImagePages;
    uint8_t Reserved_page;
    uint8_t RedMaskSize;
    uint8_t RedMaskPos;
    uint8_t GreenMaskSize;
    uint8_t GreenMaskPos;
    uint8_t BlueMaskSize;
    uint8_t BlueMaskPos;
    uint8_t ReservedMaskSize;
    uint8_t ReservedMaskPos;
    uint8_t DirectColorModeInfo;
    uint32_t PhysBasePtr;
    uint32_t OffScreenMemOffset;
    uint16_t OffScreenMemSize;
    uint8_t Reserved[206];
} __attribute__((packed)) VesaModeInfo;
