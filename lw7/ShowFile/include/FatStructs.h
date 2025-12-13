#ifndef FAT_STRUCTS_H
#define FAT_STRUCTS_H

#include <cstdint>

#pragma pack(push, 1)

struct BootSector
{
    uint8_t jmpBoot[3];
    uint8_t oemName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numFATs;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t media;
    uint16_t sectorsPerFAT16;
    uint16_t sectorsPerTrack;
    uint16_t numHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    uint32_t sectorsPerFAT32;
    uint16_t extendedFlags;
    uint16_t fsVersion;
    uint32_t rootCluster;
    uint16_t fsInfoSector;
    uint16_t backupBootSector;
    uint8_t reserved[12];
    uint8_t driveNumber;
    uint8_t reserved1;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t fsType[8];
};

struct FatDirEntry
{
    uint8_t name[11];
    uint8_t attr;
    uint8_t ntrRes;
    uint8_t crtTimeTenth;
    uint16_t crtTime;
    uint16_t crtDate;
    uint16_t lstAccDate;
    uint16_t fstClusHI;
    uint16_t wrtTime;
    uint16_t wrtDate;
    uint16_t fstClusLO;
    uint32_t fileSize;
};

struct FatLfnEntry
{
    uint8_t ord;
    uint16_t name1[5];
    uint8_t attr;
    uint8_t type;
    uint8_t chksum;
    uint16_t name2[6];
    uint16_t fstClusLO;
    uint16_t name3[2];
};

#pragma pack(pop)

#endif // FAT_STRUCTS_H
