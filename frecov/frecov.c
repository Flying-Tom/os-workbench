#include <assert.h>

struct fat_header
{
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint32_t BPB_BytsPerSec : 16;
    uint32_t BPB_SecPerClus : 8;
    uint32_t BPB_RsvdSecCnt : 16;
    uint8_t padding[420];
    uint16_t signature;
} __attribute__((packed));

int main(int argc, char *argv[])
{
    assert(sizeof(fat_header) == 512);
}
