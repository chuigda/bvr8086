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

static VesaModeInfo s_modeInfo;
static uint8_t *s_framebuffer = NULL;

static int GetVesaInfo(VesaInfo *pInfo);
static int GetModeInfo(uint16_t mode, VesaModeInfo *pInfo);
static bool IsModeCompatible(VesaModeInfo const *pInfo, uint16_t width,
                             uint16_t height, uint8_t bpp);
static int FindVesaMode(uint16_t width, uint16_t height, uint8_t bpp);
static int SetVesaMode(uint16_t mode);
static void SetVesaBank(int bank);
static void SetPalette332(void);

int16_t BvrDetectGraphics(BvrGraphicsMode *pMode, uint16_t nMode,
                          uint16_t offset) {
    VesaInfo vesaInfo;
    if (GetVesaInfo(&vesaInfo) != 0) return -1;
    
    uint16_t modeList[512];
    int numModes = 0;
    
    uint32_t modePtr = ((vesaInfo.VideoModePtr & 0xFFFF0000) >> 12) + 
                       (vesaInfo.VideoModePtr & 0xFFFF);
    
    while (_farpeekw(_dos_ds, modePtr) != 0xFFFF && numModes < 512) {
        modeList[numModes++] = _farpeekw(_dos_ds, modePtr);
        modePtr += 2;
    }
    
    int found = 0;
    for (int i = 0; i < numModes && found < nMode; i++) {
        VesaModeInfo modeInfo;
        if (GetModeInfo(modeList[i], &modeInfo) != 0) continue;
        
        if ((modeInfo.ModeAttributes & 0x19) != 0x19) continue;
        if (modeInfo.NumberOfPlanes != 1) continue;
        
        uint8_t bpp = modeInfo.BitsPerPixel;
        if (bpp != 8 && bpp != 15 && bpp != 16 && bpp != 24 && bpp != 32) continue;
        
        if (bpp == 8 && modeInfo.MemoryModel != 4) continue;
        if (bpp >= 15 && modeInfo.MemoryModel != 6) continue;
        
        if (offset > 0) {
            offset--;
            continue;
        }
        
        pMode[found].modeId = modeList[i];
        pMode[found].width = modeInfo.XResolution;
        pMode[found].height = modeInfo.YResolution;
        pMode[found].colorDepth = bpp;
        found++;
    }
    
    return found;
}

BvrFramebuffer* BvrInitGraphics(BvrGraphicsMode const *pMode) {
    int modeNum = FindVesaMode(pMode->width, pMode->height, pMode->colorDepth);
    if (modeNum == 0) return NULL;
    
    if (SetVesaMode(modeNum) != 0) return NULL;
    
    if (pMode->colorDepth == 8) {
        SetPalette332();
    }
    
    uint32_t bufferSize = pMode->width * pMode->height * 
                         ((pMode->colorDepth + 7) / 8);
    
    s_framebuffer = (uint8_t*)malloc(sizeof(BvrFramebuffer) + bufferSize);
    if (s_framebuffer == NULL) return NULL;
    
    BvrFramebuffer *fb = (BvrFramebuffer*)s_framebuffer;
    fb->mode = *pMode;
    
    memset(fb->framebuffer, 0, bufferSize);
    
    return fb;
}

void BvrSwapBuffers(BvrFramebuffer const *pFramebuffer) {
    if (pFramebuffer == NULL) return;
    
    uint32_t bytesPerPixel = (pFramebuffer->mode.colorDepth + 7) / 8;
    uint32_t screenSize = pFramebuffer->mode.width * 
                         pFramebuffer->mode.height * bytesPerPixel;
    
    uint32_t bankSize = s_modeInfo.WinSize * 1024;
    uint32_t bankGranularity = s_modeInfo.WinGranularity * 1024;
    int bankNumber = 0;
    uint32_t remaining = screenSize;
    uint8_t const *src = (uint8_t const*)pFramebuffer->framebuffer;
    
    while (remaining > 0) {
        SetVesaBank(bankNumber);
        
        uint32_t copySize = (remaining > bankSize) ? bankSize : remaining;
        dosmemput(src, copySize, 0xA0000);
        
        remaining -= copySize;
        src += copySize;
        bankNumber += bankSize / bankGranularity;
    }
}

void BvrCloseGraphics(void) {
    __dpmi_regs r;
    r.x.ax = 0x0003;
    __dpmi_int(0x10, &r);
    
    if (s_framebuffer != NULL) {
        free(s_framebuffer);
        s_framebuffer = NULL;
    }
}

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
    
    if (r.h.ah) return -1;
    
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
    
    if (r.h.ah) return -1;
    
    dosmemget(dosbuf, sizeof(VesaModeInfo), pInfo);
    return 0;
}

static bool IsModeCompatible(VesaModeInfo const *pInfo, 
                            uint16_t width, uint16_t height, uint8_t bpp) {
    if ((pInfo->ModeAttributes & 0x19) != 0x19) return false;
    if (pInfo->XResolution != width) return false;
    if (pInfo->YResolution != height) return false;
    if (pInfo->NumberOfPlanes != 1) return false;
    if (pInfo->BitsPerPixel != bpp) return false;
    
    if (bpp == 8) {
        if (pInfo->MemoryModel != 4) return false;
    } else if (bpp == 15 || bpp == 16 || bpp == 24 || bpp == 32) {
        if (pInfo->MemoryModel != 6) return false;
    } else {
        return false;
    }
    
    return true;
}

static int FindVesaMode(uint16_t width, uint16_t height, uint8_t bpp) {
    VesaInfo vesaInfo;
    uint16_t modeList[256];
    int numModes = 0;
    
    if (GetVesaInfo(&vesaInfo) != 0) return 0;
    
    uint32_t modePtr = ((vesaInfo.VideoModePtr & 0xFFFF0000) >> 12) + 
                       (vesaInfo.VideoModePtr & 0xFFFF);
    
    while (_farpeekw(_dos_ds, modePtr) != 0xFFFF) {
        modeList[numModes++] = _farpeekw(_dos_ds, modePtr);
        modePtr += 2;
    }
    
    for (int i = 0; i < numModes; i++) {
        VesaModeInfo modeInfo;
        if (GetModeInfo(modeList[i], &modeInfo) != 0) continue;
        if (IsModeCompatible(&modeInfo, width, height, bpp)) {
            memcpy(&s_modeInfo, &modeInfo, sizeof(VesaModeInfo));
            return modeList[i];
        }
    }
    
    return 0;
}

static int SetVesaMode(uint16_t mode) {
    __dpmi_regs r;
    r.x.ax = 0x4F02;
    r.x.bx = mode;
    __dpmi_int(0x10, &r);
    return r.h.ah ? -1 : 0;
}

static void SetVesaBank(int bank) {
    __dpmi_regs r;
    r.x.ax = 0x4F05;
    r.x.bx = 0;
    r.x.dx = bank;
    __dpmi_int(0x10, &r);
}

static void SetPalette332(void) {
    uint8_t palette[768];
    
    for (int i = 0; i < 256; i++) {
        uint8_t r = (i >> 5) & 0x07;
        uint8_t g = (i >> 2) & 0x07;
        uint8_t b = i & 0x03;
        
        palette[i * 3 + 0] = (r * 255) / 7;
        palette[i * 3 + 1] = (g * 255) / 7;
        palette[i * 3 + 2] = (b * 255) / 3;
    }
    
    long dosbuf = __tb & 0xFFFFF;
    dosmemput(palette, 768, dosbuf);
    
    __dpmi_regs r;
    r.x.ax = 0x4F09;
    r.x.bx = 0;
    r.x.cx = 256;
    r.x.dx = 0;
    r.x.es = (dosbuf >> 4) & 0xFFFF;
    r.x.di = dosbuf & 0xF;
    __dpmi_int(0x10, &r);
}
