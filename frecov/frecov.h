#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define _Log(format, ...)                            \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define _panic(cond, format, ...)                                                                      \
    if (!(cond)) {                                                                                     \
        printf("\33[1;31m[%s,%d,%s] " format "\33[0m\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        assert(cond);                                                                                  \
    }

#ifdef DEBUG
#define Log(...) _Log(__VA_ARGS__)
#define panic(...) _panic(__VA_ARGS__)
#else
#define Log(...)
#define panic(...)
#endif

typedef struct fat_header {
    uint8_t BS_jmpBoot[3]; // Intel jump instruction to boot code
    uint8_t BS_OEMName[8]; // OEM Name String
    uint32_t BPB_BytsPerSec : 16; // Bytes per sector
    uint32_t BPB_SecPerClus : 8; // Sectors per cluster
    uint32_t BPB_RsvdSecCnt : 16; // Reserved sectors
    uint8_t BPB_NumFATs; // Number of FATs
    uint16_t BPB_RootEntCnt; // Root Entry Count
    uint16_t BPB_TotSec16; // 16-bit Count of sectors
    uint8_t BPB_Media; // Media Type ID
    uint16_t BPB_FATSz16; // Sectors per FAT
    uint16_t BPB_SecPerTrk; // Sectors per track
    uint16_t BPB_NumHeads; // Number of heads
    uint32_t BPB_HiddSec; // Number of hidden sectors
    uint32_t BPB_TotSec32; // 32-bit Count of sectors
    uint32_t BPB_FATSz32; // 32-bit Count of sectors
    uint16_t BPB_ExtFlags; // Extended flags
    uint16_t BPB_FSVer; // Extended flags
    uint32_t BPB_RootClus; // Root directory start cluster
    uint16_t BPB_FSInfo; // File system information sector
    uint16_t BPB_BkBootSec; // File system information sector
    uint8_t BPB_Reserved[12]; // Reserved
    uint8_t BS_DrvNum; // Physical drive Number
    uint8_t BS_Reserved1; // Reserved
    uint8_t BS_BootSig; // Extended boot signature
    uint32_t BS_VolID; // Extended boot signature
    uint8_t BS_VolLab[11]; // Volume label
    uint8_t BS_FilSysType[8]; // Informational FS Type
    uint8_t padding[420]; // Reserved
    uint16_t Signature_word; // Signature
} __attribute__((packed)) fat_header;

typedef struct bmp_header {
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t bfReserved;
    uint32_t bfOffBits;

    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

} __attribute__((packed)) bmp_header;