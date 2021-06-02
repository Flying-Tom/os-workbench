#include <stdio.h>
#include <assert.h>
#include <stdint.h>

typedef struct fat_header
{
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint32_t BPB_BytsPerSec : 16;
    uint32_t BPB_SecPerClus : 8;
    uint32_t BPB_RsvdSecCnt : 16;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
    uint8_t padding[420];
    uint16_t Signature_word;
} __attribute__((packed)) fat_header;

int main(int argc, char *argv[])
{
    printf("sizeof(fat_header):%d\n", sizeof(fat_header));
    assert(sizeof(fat_header) == 512);
}
