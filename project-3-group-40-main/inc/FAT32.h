#ifndef FAT32_H
#define FAT32_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct __attribute__((packed))
{
    uint8_t BS_jmpBoot[3];
    char BS_OEMName[8];
    uint16_t BPB_BytsPerSec; // bytes per sector
    uint8_t BPB_SecPerClus;  // sectors per cluster
    uint16_t BPB_RsvdSecCnt; // number of reserved sectors in the Reserved region
    uint8_t BPB_NumFATs;     // number of FATs
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32; // total sectors in the volume
    uint32_t BPB_FATSz32;  // size of one FAT
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus; // First cluster of root directory
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    char BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    char BS_VolLab[11];
    char BS_FilSysType[8];
} BPBLOCK;

typedef struct
{
    char path[4096];
} CWD;

typedef struct
{
    int size;
    char **items;
} tokenlist;

typedef struct __attribute__((packed))
{
    unsigned char DIR_Name[11];
    uint8_t DIR_Attr;
    unsigned char DIR_NTRes;

    unsigned char padding1;
    unsigned short padding2;
    unsigned short padding3;
    unsigned short padding4;

    uint16_t DIR_FstClusHI;

    unsigned short padding5;
    unsigned short padding6;

    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} DIR;

typedef struct
{
    char fileName[11];
    uint32_t currFilePos;
    uint32_t currFilePosOffset;
    int mode;
    uint32_t offset;
    uint32_t fileSize;
    char *filePath;
} OpenFile;

FILE *ImgFile;
BPBLOCK BPB;
int FirstDataSector;
CWD cwd;
int CurrentDirectory;
int CurrentFileCluster;
uint32_t FirstFatSector;
uint32_t BytesPerCluster;
int NumOpenFiles;

OpenFile OpenedFiles[10];
tokenlist *tokenize(char *input);
void free_tokens(tokenlist *tokens);
char *GetUserInput();
void add_token(tokenlist *tokens, char *item);
void AddToPath(char *dir);
uint32_t GetClusterHiLo(unsigned short, unsigned short);

void CloseFile(char *);
void OpenCMD(char *, char *);
int ListDirectory(int);
int ChangeDirectory(char *);
void LSOF();

void LSeek(char *, uint32_t);
void Info(long);
void ReadCMD(char *, uint32_t);

void CreateFile(char *);
int findFile(char *);
void MakeDirectory(char *);
void RemoveTarget(char *);
void RemoveDirectory(char *);

#endif