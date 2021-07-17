#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
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
    uint32_t BPB_FATSz32; // 32-bit count of sectors per FAT
    uint16_t BPB_ExtFlags; // Extended flags
    uint16_t BPB_FSVer; // File system version
    uint32_t BPB_RootClus; // Root directory start cluster
    uint16_t BPB_FSInfo; // File system information sector
    uint16_t BPB_BkBootSec; // Backup boot sector
    uint8_t BPB_Reserved[12]; // Reserved
    uint8_t BS_DrvNum; // Physical drive Number
    uint8_t BS_Reserved1; // Reserved
    uint8_t BS_BootSig; // Extended boot signature
    uint32_t BS_VolID; // Volume serial number
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

typedef struct DIR {
    uint8_t DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;

} __attribute__((packed)) DIR_t;

typedef struct LDIR {
    uint8_t LDIR_Ord;
    uint16_t LDIR_Name1[5];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type;
    uint8_t LDIR_Chksum;
    uint16_t LDIR_Name2[6];
    uint16_t LDIR_FstClusLO;
    uint16_t LDIR_Name3[2];

} __attribute__((packed)) LDIR_t;

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
/* ==========================================================*/
char name_buf[256];

unsigned char ChkSum(unsigned char* pFcbName)
{
    short FcbNameLen;
    unsigned char Sum;
    Sum = 0;
    for (FcbNameLen = 11; FcbNameLen != 0; FcbNameLen--) {
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
    }
    return (Sum);
}

int main(int argc, char* argv[])
{

    panic(argc == 2, "Wrong args : argc != 2");
    char* disk_path = argv[1];
    FILE* fp = fopen(disk_path, "rb");
    panic(fp, "Fail to open the disk :%s", disk_path);
    fseek(fp, 0, SEEK_END);
    uint32_t img_size = ftell(fp);
    int fd = open(disk_path, O_RDONLY);

    uintptr_t img_addr = (uintptr_t)mmap(NULL, img_size, PROT_READ, MAP_SHARED, fd, 0);
    fat_header* disk = (fat_header*)img_addr;

    panic(strncmp((char*)disk->BS_FilSysType, "FAT32", 5) == 0, "FAT header Error | BS_FilSysTypem : %s", (char*)disk->BS_FilSysType);
    panic(disk->Signature_word == 0xaa55, "FAT header Error | Signature_word : %x ", disk->Signature_word);

    void* cluster_addr = (void*)(img_addr + (disk->BPB_RsvdSecCnt + disk->BPB_NumFATs * disk->BPB_FATSz32 + disk->BPB_HiddSec) * disk->BPB_BytsPerSec);
    close(fd);
    fclose(fp);

    uintptr_t addr;
    for (addr = (uintptr_t)cluster_addr; addr < img_addr + img_size; addr += sizeof(DIR_t)) {
        DIR_t* dir = (DIR_t*)addr;
        if (dir->DIR_Attr == ATTR_LONG_NAME)
            continue;

        if ((strncmp((char*)(dir->DIR_Name + 8), "bmp", 3) == 0 || strncmp((char*)(dir->DIR_Name + 8), "BMP", 3) == 0) && dir->DIR_NTRes == 0) {
            //printf("%s\n", dir->DIR_Name);

            memset(name_buf, '\0', sizeof(name_buf));
            bool isLDIR = false;
            for (int i = 0; i < 7; i++) {
                if (!isdigit(dir->DIR_Name[i]) && !isupper(dir->DIR_Name[i]) && !islower(dir->DIR_Name[i])) {
                    isLDIR = true;
                    break;
                }
            }
            if (isLDIR)
                continue;

            unsigned char chksum = ChkSum((unsigned char*)dir->DIR_Name);

            LDIR_t* ldir = (LDIR_t*)(addr - sizeof(DIR_t));
            if (ldir->LDIR_Attr != ATTR_LONG_NAME)
                continue;

            int name_buf_cnt = 0;
            for (int cnt = 1, i = 1; (uintptr_t)(ldir) >= (uintptr_t)cluster_addr && i <= 10; i++) {
                if (ldir->LDIR_Chksum == chksum && (ldir->LDIR_Ord & 0x0f) == cnt) {
                    cnt++;
                    for (int j = 0; j < 5; j++)
                        name_buf[name_buf_cnt++] = ldir->LDIR_Name1[j];
                    for (int j = 0; j < 6; j++)
                        name_buf[name_buf_cnt++] = ldir->LDIR_Name2[j];
                    for (int j = 0; j < 2; j++)
                        name_buf[name_buf_cnt++] = ldir->LDIR_Name3[j];
                }
                if ((ldir->LDIR_Ord & 0x40) && ldir->LDIR_Chksum == chksum)
                    break;
                else
                    ldir--;
            }

            printf("%d %s\n", name_buf_cnt, name_buf);
            /*
            for (int i = 0; i < 13; i++)
                printf("%c", name_buf[i]);
            printf("\n");
            */
        }
    }
}

__attribute__((constructor)) void struct_check()
{
    panic(sizeof(fat_header) == 512, "bad fat_header");
    panic(sizeof(bmp_header) == 0x36, "bad bmp_header");
    panic(sizeof(DIR_t) == 32, "bad DIR");
    panic(sizeof(LDIR_t) == 32, "bad LDIR");
    //printf("%c", 0xe5);
    //printf("hello world\n");
}