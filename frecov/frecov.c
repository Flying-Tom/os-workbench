#include "frecov.h"

int main(int argc, char *argv[])
{
    panic(sizeof(fat_header) == 512, "bad fat_header");
    panic(sizeof(bmp_header) == 0x36, "bad bmp_header");
    panic(argc == 2, "Wrong args : argc != 2");
    char *disk_path = argv[1];
    FILE *fp = fopen(disk_path, "rb");
    panic(fp, "Fail to open the disk :%s", disk_path);
    fseek(fp, 0, SEEK_END);
    uint32_t disk_size = ftell(fp);
    int fd = open(disk_path, O_RDONLY);

    fat_header *disk = mmap(NULL, disk_size, PROT_READ, MAP_SHARED, fd, 0);
    panic(strncmp((char *)disk->BS_FilSysType, "FAT32", 5) == 0, "FAT header Error | BS_FilSysTypem : %s", (char *)disk->BS_FilSysType);
    panic(disk->Signature_word == 0xaa55, "FAT header Error | Signature_word : %x ", disk->Signature_word);

    fclose(fp);
}
