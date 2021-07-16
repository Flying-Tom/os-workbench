#include "frecov.h"

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
            bool isLDIR = false;
            for (int i = 0; i < 7; i++) {
                if (!isdigit(dir->DIR_Name[i]) && !isupper(dir->DIR_Name[i]) && !islower(dir->DIR_Name[i])) {
                    isLDIR = true;
                    break;
                }
            }
            if (isLDIR)
                continue;
            printf("fuck\n");
        }
    }
}

__attribute__((constructor)) void struct_check()
{
    panic(sizeof(fat_header) == 512, "bad fat_header");
    panic(sizeof(bmp_header) == 0x36, "bad bmp_header");
    panic(sizeof(DIR_t) == 32, "bad DIR");
    panic(sizeof(LDIR_t) == 32, "bad LDIR");
    //printf("hello world\n");
}