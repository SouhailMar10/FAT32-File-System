

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "FAT32.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <fat32 image>\n", argv[0]);
        return 1;
    }

    ImgFile = fopen(argv[1], "r+");
    fread(&BPB, sizeof(BPBLOCK), 1, ImgFile);

    BytesPerCluster = BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus;
    FirstFatSector = BPB.BPB_RsvdSecCnt;
    FirstDataSector = BPB.BPB_RsvdSecCnt + (BPB.BPB_NumFATs * BPB.BPB_FATSz32);
    AddToPath(argv[1]);

    for (int i = 0; i < 10; i++)
    {
        OpenedFiles[i].mode = 0;
    }

    char *input;
    long offset = 0;
    CurrentDirectory = BPB.BPB_RootClus;

    while (1)
    {
        printf("%s/>", cwd.path);
        input = GetUserInput();
        tokenlist *tokens = tokenize(input);
        if (strcmp(tokens->items[0], "exit") == 0)
        {
            return 0;
        }
        else if (strcmp(tokens->items[0], "info") == 0)
        {
            offset = BPB.BPB_BytsPerSec * BPB.BPB_FSInfo;
            fseek(ImgFile, offset, SEEK_SET);

            printf("Bytes Per Sector: %d\n", BPB.BPB_BytsPerSec);
            printf("Sectors Per Cluster: %d\n", BPB.BPB_SecPerClus);
            printf("Root Cluster: %d\n", BPB.BPB_RootClus);
            printf("Total Clusters in Data: %d\n", BPB.BPB_TotSec32);
            printf("Entries per FAT: %d\n", ((BPB.BPB_FATSz32 * BPB.BPB_BytsPerSec) / 4));
            printf("Image Size: %d\n", BPB.BPB_TotSec32 * BPB.BPB_BytsPerSec);
         
        }
        else if (strcmp(tokens->items[0], "ls") == 0)
        {
            int ls = 0;
            ls = ListDirectory(CurrentDirectory);
        }
        else if (strcmp(tokens->items[0], "cd") == 0)
        {
            if (tokens->items[1] != NULL)
            {
                CurrentDirectory = ChangeDirectory(tokens->items[1]);
            }
        }
        else if (strcmp(tokens->items[0], "creat") == 0)
        {
            CreateFile(tokens->items[1]);
        }
        else if (strcmp(tokens->items[0], "mkdir") == 0)
        {
            if (tokens->items[1] != NULL)
            {
                MakeDirectory(tokens->items[1]);
            }
        }
        else if (strcmp(tokens->items[0], "rm") == 0)
        {
            RemoveTarget(tokens->items[1]);
        }
        else if (strcmp(tokens->items[0], "rmdir") == 0)
        {
            if (tokens->items[1] != NULL)
            {
                RemoveDirectory(tokens->items[1]);
            }
        }
        else if (strcmp(tokens->items[0], "open") == 0)
        {
            if (tokens->items[1] == NULL)
            {
                printf("Enter valid filename\n");
            }
            else if (tokens->items[2] == NULL)
            {
                printf("%s%s", "Enter a valid value for file operation.",
                       " Either -r -w -rw or-wr\n");
            }
            else if (!strcmp(tokens->items[2], "-r") || !strcmp(tokens->items[2], "-w") || !strcmp(tokens->items[2], "-rw") || !strcmp(tokens->items[2], "-wr"))
            {
                OpenCMD(tokens->items[1], tokens->items[2]);
            }
            else
            {
                printf("%s%s", "Enter a valid value for file operation.",
                       " Either -r -w -rw or-wr\n");
            }
        }
        else if (strcmp(tokens->items[0], "close") == 0)
        {
            if (tokens->items[1] == NULL)
            {
                printf("Enter a valid filename\n");
            }
            else
            {
                CloseFile(tokens->items[1]);
            }
        }
        else if (strcmp(tokens->items[0], "lsof") == 0)
        {
            LSOF();
        }
        else if (strcmp(tokens->items[0], "lseek") == 0)
        {
            if (tokens->items[1] == NULL || tokens->items[2] == NULL)
            {
                printf("Enter valid values for lseek command\n");
            }
            else
            {
                LSeek(tokens->items[1], atoi(tokens->items[2]));
            }
        }
        else if (strcmp(tokens->items[0], "read") == 0)
        {
            if (tokens->items[1] == NULL)
            {
                printf("Enter a valid file name\n");
            }
            else
            {
                ReadCMD(tokens->items[1], atoi(tokens->items[2]));
            }
        }
        else if (strcmp(tokens->items[0], "write") == 0)
        {
            if (tokens->items[1] == NULL)
            {
                printf("Enter a valid filename\n");
            }
            else if (tokens->items[2] == NULL)
            {
                printf("Enter a valid value to write into the file\n");
            }
            else
            {
                WriteCMD(tokens->items[1], tokens->items[2]);
            }
        }
        else
        {
            printf("Invalid command\n");
        }
        free(input);
        free_tokens(tokens);
    }
    return 0;
}

uint32_t GetClusterHiLo(unsigned short hi, unsigned short lo)
{
    return ((hi << 8) | lo);
}

int FatEntryOffset(int cluster)
{
    int FatDir_EntryOffset = (FirstFatSector * BPB.BPB_BytsPerSec) + (cluster * 4);
    return FatDir_EntryOffset;
}

int GetNextCluster(int cluster)
{
    fseek(ImgFile, FatEntryOffset(cluster), SEEK_SET);
    int nextCluster;
    fread(&nextCluster, sizeof(int), 1, ImgFile);
    return nextCluster;
}

int ClusterByteOffset(int cluster)
{
    int clus = (FirstDataSector + ((cluster - 2) * BPB.BPB_SecPerClus)) * BPB.BPB_BytsPerSec;
    return clus;
}

int FAT32_Format(int cluster)
{
    int NextCluster = (FatEntryOffset(cluster)) + 4;
    fseek(ImgFile, NextCluster, SEEK_SET);
    int next_chain;
    fread(&next_chain, sizeof(int), 1, ImgFile);
    if (next_chain < 0x0FFFFFF8)
    {
        return next_chain;
    }
    else
    {
        return 0;
    }
}

int AllocateCluster(int cluster)
{
    uint32_t i_pos = ftell(ImgFile);
    uint32_t value;
    int ctr = 0;
    uint32_t found = 0xFFFFFFFF;
    fseek(ImgFile, FatEntryOffset(0), SEEK_SET);
    do
    {
        fread(&value, sizeof(uint32_t), 1, ImgFile);
        ctr++;
    } while (value != 0);
    ctr -= 1;
    fseek(ImgFile, ftell(ImgFile) - 4, SEEK_SET);
    fwrite(&found, sizeof(uint32_t), 1, ImgFile);
    fseek(ImgFile, i_pos, SEEK_SET);

    return ctr;
}

int FAT_Chain_Ext(int cluster)
{
    int NextCluster = FatEntryOffset(cluster);
    fseek(ImgFile, NextCluster, SEEK_SET);
    int newclust = AllocateCluster(cluster);

    while (NextCluster != -1)
    {
        fread(&NextCluster, sizeof(int), 1, ImgFile);
    }
    fseek(ImgFile, ftell(ImgFile) - 4, SEEK_SET);
    fwrite(&newclust, sizeof(int), 1, ImgFile);
    return newclust;
}

void OpenCMD(char *token1, char *token2)
{
    int NextCluster = CurrentDirectory;
    DIR Dir_Entry;
    if (NumOpenFiles == 10)
    {
        printf("Error: too many files are opened\n");
        return;
    }

    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            if (Dir_Entry.DIR_Attr == 0x20)
            {
                for (int k = 0; k < 10; k++)
                {
                    if (!strcmp(OpenedFiles[k].fileName, token1) && OpenedFiles[k].mode != 0)
                    {
                        printf("Error: file %s is already opened\n", token1);
                        return;
                    }
                    if (OpenedFiles[k].mode == 0)
                    {

                        for (int i = 0; i < 11; i++)
                        {
                            if (Dir_Entry.DIR_Name[i] == 0x20)
                                Dir_Entry.DIR_Name[i] = 0x00;
                        }

                        if (!strcmp(token1, Dir_Entry.DIR_Name))
                        {
                            strcpy(OpenedFiles[k].fileName, Dir_Entry.DIR_Name);
                            OpenedFiles[k].currFilePos = GetClusterHiLo(Dir_Entry.DIR_FstClusHI, Dir_Entry.DIR_FstClusLO);
                            if (OpenedFiles[k].currFilePos == 0)
                            {
                                OpenedFiles[k].currFilePos = BPB.BPB_RootClus;
                            }
                            OpenedFiles[k].currFilePosOffset = ClusterByteOffset(OpenedFiles[k].currFilePos);
                            if (!strcmp(token2, "-r"))
                            {
                                OpenedFiles[k].mode = 1;
                            }
                            if (!strcmp(token2, "-w"))
                            {
                                OpenedFiles[k].mode = 2;
                            }
                            if (!strcmp(token2, "-rw"))
                            {
                                OpenedFiles[k].mode = 3;
                            }
                            if (!strcmp(token2, "-wr"))
                            {
                                OpenedFiles[k].mode = 4;
                            }
                            OpenedFiles[k].fileSize = Dir_Entry.DIR_FileSize;
                            OpenedFiles[k].offset = 0;
                            OpenedFiles[k].filePath = cwd.path;
                            printf("opened %s\n", OpenedFiles[k].fileName);
                            NumOpenFiles += 1;
                            break;
                        }
                    }
                }
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
    return;
}

void CloseFile(char *token)
{
    int NextCluster = CurrentDirectory;

    DIR Dir_Entry;
    int i;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            for (int j = 0; j < 10; j++)
            {
                if (!strcmp(token, OpenedFiles[j].fileName))
                {
                    if (OpenedFiles[j].mode == 0)
                    {
                        printf("Error: file %s is not opened\n", OpenedFiles[j].fileName);
                        return;
                    }
                    else
                    {
                        OpenedFiles[j].mode = 0;
                        printf("File %s is closed\n", OpenedFiles[j].fileName);
                        NumOpenFiles--;
                        return;
                    }
                }
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
}

void LSOF()
{
    if (NumOpenFiles == 0)
    {
        printf("No Opened Files\n");
        return;
    }
    else
    {
        printf("INDEX NAME      MODE      OFFSET      PATH\n");
        for (int i = 0; i < 10; i++)
        {
            if (OpenedFiles[i].mode != 0)
            {
                if (OpenedFiles[i].mode == 1)
                {
                    printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "r", OpenedFiles[i].offset, OpenedFiles[i].filePath);
                }
                if (OpenedFiles[i].mode == 2)
                {
                    printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "w", OpenedFiles[i].offset, OpenedFiles[i].filePath);
                }
                if (OpenedFiles[i].mode == 3)
                {
                    printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "rw", OpenedFiles[i].offset, OpenedFiles[i].filePath);
                }
                if (OpenedFiles[i].mode == 4)
                {
                    printf("%-6d%-10s%-10s%-12d%-s\n", i, OpenedFiles[i].fileName, "wr", OpenedFiles[i].offset, OpenedFiles[i].filePath);
                }
            }
        }
    }
}

void sizeCmd(char *token)
{
    int NextCluster = CurrentDirectory;
    DIR Dir_Entry;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (!strcmp(Dir_Entry.DIR_Name, token))
            {
                if (Dir_Entry.DIR_Attr == 0x20)
                {
                    printf("%d %s\n", Dir_Entry.DIR_FileSize, Dir_Entry.DIR_Name);
                    return;
                }
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
}

void LSeek(char *token, uint32_t o)
{
    int NextCluster = CurrentDirectory;

    DIR Dir_Entry;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (!strcmp(Dir_Entry.DIR_Name, token))
            {
                for (int i = 0; i < 10; i++)
                {
                    if (!strcmp(Dir_Entry.DIR_Name, OpenedFiles[i].fileName) && OpenedFiles[i].mode == 0)
                    {
                        return;
                    }
                    if (!strcmp(Dir_Entry.DIR_Name, OpenedFiles[i].fileName) && OpenedFiles[i].mode != 0)
                    {
                        if (o > Dir_Entry.DIR_FileSize)
                        {
                            printf("Error: offset is greater than file size\n");
                        }
                        else
                        {
                            OpenedFiles[i].offset = o;
                        }
                        return;
                    }
                }
                printf("Error: file %s is not opened\n", token);
                return;
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
}

void ReadCMD(char *token, uint32_t token2)
{
    int NextCluster = CurrentDirectory;

    DIR Dir_Entry;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (!strcmp(Dir_Entry.DIR_Name, token))
            {
                for (int i = 0; i < 10; i++)
                {
                    if (!strcmp(Dir_Entry.DIR_Name, OpenedFiles[i].fileName))
                    {
                        if (OpenedFiles[i].mode == 2)
                        {
                            printf("Error: file %s must be opened in -r -rw or -wr mode\n, OpenedFiles[i].fileName");
                            break;
                        }
                        else if (OpenedFiles[i].mode == 0)
                        {
                            printf("Error: file %s is not opened\n", OpenedFiles[i].fileName);
                            break;
                        }

                        if (token2 >= OpenedFiles[i].fileSize)
                        {

                            fseek(ImgFile,
                                  OpenedFiles[i].currFilePosOffset + OpenedFiles[i].offset, SEEK_SET);
                            for (int j = 0; j < BytesPerCluster; j++)
                            {
                                char byte = fgetc(ImgFile);
                                printf("%c", byte);
                            }
                            OpenedFiles[i].offset = OpenedFiles[i].fileSize;
                            int back = FAT32_Format(
                                OpenedFiles[i].currFilePos);
                            while (back != 0)
                            {
                                fseek(ImgFile, ClusterByteOffset(back),
                                      SEEK_SET);
                                for (int j = 0; j < BytesPerCluster; j++)
                                {
                                    char byte = fgetc(ImgFile);
                                    printf("%c", byte);
                                    OpenedFiles[i].offset =
                                        OpenedFiles[i].offset + 1;
                                    printf("files offset: %d\n",
                                           OpenedFiles[i].offset);
                                }
                                back = FAT32_Format(back);
                            }
                            return;
                        }
                        else
                        {
                            int counter = 0;
                            while (counter < token2)
                            {
                                fseek(ImgFile, OpenedFiles[i].currFilePosOffset + OpenedFiles[i].offset, SEEK_SET);
                                for (int j = 0; j < BytesPerCluster; j++)
                                {
                                    char byte = fgetc(ImgFile);
                                    printf("%c", byte);
                                    counter++;
                                    OpenedFiles[i].offset = OpenedFiles[i].offset + 1;
                                    if (!(counter < token2))
                                        return;
                                }
                                int back = FAT32_Format(OpenedFiles[i].currFilePos);
                                while (back != 0)
                                {
                                    fseek(ImgFile, ClusterByteOffset(back), SEEK_SET);
                                    for (int j = 0; j < BytesPerCluster; j++)
                                    {
                                        char byte = fgetc(ImgFile);
                                        printf("%c", byte);
                                        counter++;
                                        if (!(counter < token2))
                                        {
                                            OpenedFiles[i].offset = counter;
                                            return;
                                        }
                                    }
                                    back = FAT32_Format(back);
                                }
                                return;
                            }
                        }
                    }
                }
                break;
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
}

int findFile(char *file)
{
    int NextCluster = CurrentDirectory;

    DIR Dir_Entry;
    int i;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (!strcmp(Dir_Entry.DIR_Name, file))
            {
                fseek(ImgFile, ClusterByteOffset(CurrentDirectory), SEEK_SET);
                return 1;
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
    return -1;
}

void WriteCMD(char *token1, char *token2)
{
    int NextCluster = CurrentDirectory;
    DIR Dir_Entry;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (Dir_Entry.DIR_Attr == 0x20)
            {
                if (!strcmp(Dir_Entry.DIR_Name, token1))
                {
                    for (int j = 0; j < 10; j++)
                    {
                        if (!strcmp(OpenedFiles[j].fileName, Dir_Entry.DIR_Name))
                        {
                            if (OpenedFiles[j].mode == 1)
                            {
                                printf("Error: file %s must be opened to write to\n", OpenedFiles[j].fileName);
                                return;
                            }
                            else if (OpenedFiles[j].mode == 0)
                            {
                                printf("Error: file %s is not opened\n", OpenedFiles[j].fileName);
                                return;
                            }
                            else
                            {
                                uint32_t sizePlusOffset = OpenedFiles[j].offset + strlen(token2);
                                printf("size plus offset is %d\n", sizePlusOffset);
                                if (sizePlusOffset > OpenedFiles[j].fileSize)
                                {
                                    printf("%s", "Error: writing past the end of the file\n");
                                    OpenedFiles[j].fileSize += strlen(token2);
                                    printf("File Pos: %d\n", ftell(ImgFile));
                                    fseek(ImgFile, OpenedFiles[j].currFilePosOffset + OpenedFiles[j].offset, SEEK_SET);
                                    printf("File position: %d\n", ftell(ImgFile));
                                    char buffer[strlen(token2)];
                                    strcpy(buffer, token2);
                                    for (int k = 0; k < strlen(token2); k++)
                                    {
                                        fwrite(&buffer[k], sizeof(char), 1, ImgFile);
                                        OpenedFiles[j].offset++;
                                        if (OpenedFiles[j].offset % BytesPerCluster == 0)
                                        {
                                            if (FAT32_Format(OpenedFiles[j].currFilePos) != 0)
                                            {
                                                fseek(ImgFile, FAT32_Format(OpenedFiles[j].currFilePos), SEEK_SET);
                                            }
                                            else
                                            {
                                                FAT_Chain_Ext(OpenedFiles[j].currFilePos);
                                                fseek(ImgFile, FAT32_Format(OpenedFiles[j].currFilePos), SEEK_SET);
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    fseek(ImgFile, OpenedFiles[j].currFilePosOffset + OpenedFiles[j].offset, SEEK_SET);
                                    char buffer[strlen(token2)];
                                    strcpy(buffer, token2);
                                    printf("buffer: %s\n", buffer);
                                    printf("length of token: %d\n", strlen(token2));
                                    printf("len of buffer: %d\n", strlen(buffer));
                                    fwrite(buffer, sizeof(char), strlen(token2), ImgFile);
                                    OpenedFiles[j].offset += strlen(token2);
                                }
                                return;
                            }
                        }
                    }
                    printf("Error: file %s is not opened\n", token1);
                    return;
                }
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
    printf("Error: file %s not found\n", token1);
}

void AddToPath(char *dir)
{
    if (dir == NULL)
    {
        return;
    }
    else if (strcmp(dir, "..") == 0)
    {
        char *last = strrchr(cwd.path, '/');
        if (last != NULL)
        {
            *last = '\0';
        }
    }
    else if (strcmp(dir, ".") != 0)
    {
        strcat(cwd.path, "/");
        strcat(cwd.path, dir);
    }
}

void free_tokens(tokenlist *tokens)
{
    for (int i = 0; i < tokens->size; i++)
        free(tokens->items[i]);
    free(tokens->items);
    free(tokens);
}

tokenlist *tokenize(char *input)
{
    int is_in_string = 0;
    tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **)malloc(sizeof(char *));
    char *token = input;
    for (; *input != '\0'; input++)
    {
        if (*input == '\"' && !is_in_string)
        {
            is_in_string = 1;
            token = input + 1;
        }
        else if (*input == '\"' && is_in_string)
        {
            *input = '\0';
            add_token(tokens, token);
            while (*(input + 1) == ' ')
            {
                input++;
            }
            token = input + 1;
            is_in_string = 0;
        }
        else if (*input == ' ' && !is_in_string)
        {
            *input = '\0';
            while (*(input + 1) == ' ')
            {
                input++;
            }
            add_token(tokens, token);
            token = input + 1;
        }
    }
    if (is_in_string)
    {
        printf("error: string not properly enclosed.\n");
        tokens->size = -1;
        return tokens;
    }

    if (*token != '\0')
    {
        add_token(tokens, token);
    }

    return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
    int i = tokens->size;
    tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *)malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;
    strcpy(tokens->items[i], item);
    tokens->size += 1;
}

char *GetUserInput()
{
    char *buf = (char *)malloc(sizeof(char) * 40);
    memset(buf, 0, 40);
    char c;
    int len = 0;
    int resizes = 1;
    int is_leading_space = 1;
    while ((c = fgetc(stdin)) != '\n' && !feof(stdin))
    {

        if (c != ' ')
        {
            is_leading_space = 0;
        }
        else if (is_leading_space)
        {
            continue;
        }
        buf[len] = c;
        if (++len >= (40 * resizes))
        {
            buf = (char *)realloc(buf, (40 * ++resizes) + 1);
            memset(buf + (40 * (resizes - 1)), 0, 40);
        }
    }
    buf[len + 1] = '\0';

    char *end = &buf[len - 1];
    while (*end == ' ')
    {
        *end = '\0';
        end--;
    }
    return buf;
}

void MakeDirectory(char *token)
{
    int NextCluster = CurrentDirectory;
    int thisDirectory = CurrentDirectory;
    DIR newDir_Entry;
    DIR Dir_Entry;
    for (int i = 0; i < 11; i++)
    {
        newDir_Entry.DIR_Name[i] = toupper(token[i]);
    }
    int loc = AllocateCluster(CurrentDirectory);
    newDir_Entry.DIR_Attr = 0x10;
    newDir_Entry.DIR_NTRes = 0;

    newDir_Entry.DIR_FstClusHI = (loc >> 16);

    newDir_Entry.DIR_FstClusLO = (loc & 0xffff);
    newDir_Entry.DIR_FileSize = 0;
    char stop = 'f';

    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        for (int i = 0; i < (BytesPerCluster / 32); i++)
        {
            int newDirLoc = ftell(ImgFile);
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            if (Dir_Entry.DIR_Attr == 0x0F)
            {
                continue;
            }
            if (Dir_Entry.DIR_Name[0] == 0 || Dir_Entry.DIR_Name[0] == 0xE5)
            {
                fseek(ImgFile, newDirLoc, SEEK_SET);
                fwrite(&newDir_Entry, sizeof(DIR), 1, ImgFile);
                stop = 't';
                break;
            }
        }
        if (stop == 't')
        {
            break;
        }
        NextCluster = GetNextCluster(NextCluster);
    }

    fseek(ImgFile, ClusterByteOffset(loc), SEEK_SET);
    DIR dot;
    dot.DIR_Attr = 0x10;
    dot.DIR_NTRes = 0;
    dot.DIR_FstClusHI = (loc >> 16);
    dot.DIR_FstClusLO = (loc & 0xffff);
    dot.DIR_FileSize = 0;
    strcpy(dot.DIR_Name, ".");
    fwrite(&dot, sizeof(DIR), 1, ImgFile);
    DIR dotdot;
    dotdot.DIR_Attr = 0x10;
    dotdot.DIR_NTRes = 0;
    dotdot.DIR_FstClusHI = (CurrentDirectory >> 16);
    dotdot.DIR_FstClusLO = (CurrentDirectory & 0xffff);
    dotdot.DIR_FileSize = 0;
    strcpy(dotdot.DIR_Name, "..");
    fwrite(&dotdot, sizeof(DIR), 1, ImgFile);
}

int ListDirectory(int Directory)
{
    int NextCluster = CurrentDirectory;
    DIR Dir_Entry;
    int i;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);

            if (Dir_Entry.DIR_Attr == 0x0F)
            {
                continue;
            }
            if (Dir_Entry.DIR_Name[0] == 0 || Dir_Entry.DIR_Name[0] == 0xE5)
            {
                continue;
            }
            printf("%s\n", Dir_Entry.DIR_Name);
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
    return 1;
}

int ChangeDirectory(char *token)
{
    int NextCluster = CurrentDirectory;

    fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);

    DIR Dir_Entry;
    int i;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            if (Dir_Entry.DIR_Attr == 0x0F)
            {
                continue;
            }
            if (Dir_Entry.DIR_Name[0] == 0 || Dir_Entry.DIR_Name[0] == 0xE5)
            {
                continue;
            }

            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (!strcmp(token, Dir_Entry.DIR_Name))
            {
                if (Dir_Entry.DIR_Attr == 0x10)
                {
                    CurrentDirectory = GetClusterHiLo(Dir_Entry.DIR_FstClusHI, Dir_Entry.DIR_FstClusLO);
                    if (CurrentDirectory == 0)
                    {
                        CurrentDirectory = BPB.BPB_RootClus;
                    }
                    AddToPath(token);
                    break;
                }
                else
                {
                    printf("Directory %s does not exist \n", token);
                }
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }

    return CurrentDirectory;
}

void DeAllocateClusterter(cluster)
{
    int temp = 0;
    int zero = 0;
    int location = 0;
    fseek(ImgFile, FatEntryOffset(cluster), SEEK_SET);
    do
    {
        location = ftell(ImgFile);
        fread(&temp, sizeof(int), 1, ImgFile);
        fseek(ImgFile, location, SEEK_SET);
        fwrite(&zero, sizeof(int), 1, ImgFile);
        fseek(ImgFile, FatEntryOffset(temp), SEEK_SET);
    } while (temp > 0x0FFFFFF8);
}

int IsEmpty(cluster)
{
    int empty = 0;
    fseek(ImgFile, ClusterByteOffset(cluster), SEEK_SET);
    DIR Dir_Entry;
    int i = 0;
    uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
    do
    {
        fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);

        for (int i = 0; i < 11; i++)
        {
            if (Dir_Entry.DIR_Name[i] == 0x20)
                Dir_Entry.DIR_Name[i] = 0x00;
        }

        if (Dir_Entry.DIR_Name[0] != '.' && Dir_Entry.DIR_Name[0] != 0xE5 && Dir_Entry.DIR_Name[0] != 0)
        {
            printf("directory not empty\n");
            return empty;
        }
    } while (++i < byteLimit);
    empty = 1;
    return empty;
}

void RemoveDirectory(char *token)
{
    if (!strcmp(token, "..") || !strcmp(token, "."))
    {
        printf("Error: Cannot delete . or .. directories\n");
        return;
    }
    DIR Dir_Entry;
    int NextCluster = CurrentDirectory;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            int NameAddress = ftell(ImgFile);
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            if (Dir_Entry.DIR_Attr == 0x0F)
            {
                continue;
            }

            if (Dir_Entry.DIR_Name[0] == 0 || Dir_Entry.DIR_Name[0] == 0xE5)
            {
                continue;
            }

            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (strcmp(Dir_Entry.DIR_Name, token) == 0)
            {
                if (Dir_Entry.DIR_Attr == 0x10)
                {
                    if (IsEmpty(GetClusterHiLo(Dir_Entry.DIR_FstClusHI, Dir_Entry.DIR_FstClusLO)) == 0)
                    {
                        printf("Error: Directory not empty\n");
                        return;
                    }

                    DeAllocateClusterter(GetClusterHiLo(Dir_Entry.DIR_FstClusHI, Dir_Entry.DIR_FstClusLO));
                    char del;
                    char *delete = &del;
                    *delete = 0xE5;
                    if (GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                    {
                        fseek(ImgFile, NameAddress, SEEK_SET);
                        Dir_Entry.DIR_Name[0] = 0xE5;
                        fwrite(&Dir_Entry, sizeof(DIR), 1, ImgFile);
                    }
                    else
                    {
                        fseek(ImgFile, NameAddress, SEEK_SET);
                        Dir_Entry.DIR_Name[0] = 0x00;

                        fwrite(delete, sizeof(char), 1, ImgFile);
                    }
                    break;
                }
                else
                {
                    printf("Target %s is not a directory\n", token);
                }
            }
        } while (i++ < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
}

void RemoveTarget(char *token)
{

    DIR Dir_Entry;
    int NextCluster = CurrentDirectory;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        int i = 0;
        uint32_t byteLimit = (BPB.BPB_BytsPerSec * BPB.BPB_SecPerClus) / 32;
        do
        {
            int NameAddress = ftell(ImgFile);
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);

            if (Dir_Entry.DIR_Attr == 0x0F)
            {
                continue;
            }
            if (Dir_Entry.DIR_Name[0] == 0 || Dir_Entry.DIR_Name[0] == 0xE5)
            {
                continue;
            }
            for (int i = 0; i < 11; i++)
            {
                if (Dir_Entry.DIR_Name[i] == 0x20)
                    Dir_Entry.DIR_Name[i] = 0x00;
            }

            if (strcmp(Dir_Entry.DIR_Name, token) == 0)
            {
                if (Dir_Entry.DIR_Attr == 0x20)
                {

                    DeAllocateClusterter(GetClusterHiLo(Dir_Entry.DIR_FstClusHI, Dir_Entry.DIR_FstClusLO));
                    char *delete = 0xE5;
                    if (GetNextCluster(CurrentDirectory) < 0x0FFFFFF8)
                    {
                        fseek(ImgFile, NameAddress, SEEK_SET);
                        Dir_Entry.DIR_Name[0] = 0xE5;
                        fwrite(&Dir_Entry, sizeof(DIR), 1, ImgFile);
                    }
                    else
                    {
                        fseek(ImgFile, NameAddress, SEEK_SET);
                        Dir_Entry.DIR_Name[0] = 0x00;

                        fwrite(delete, sizeof(char), 1, ImgFile);
                    }
                    break;
                }
                else
                {
                    printf("Target %s is not a directory\n", token);
                }
            }
        } while (++i < byteLimit);
        NextCluster = GetNextCluster(NextCluster);
    }
}

void CreateFile(char *filename)
{

    if (findFile(filename) == 1)
    {
        printf("Error: %s already exists\n", filename);
        return;
    }

    int NextCluster = CurrentDirectory;
    DIR newDir_Entry;
    DIR Dir_Entry;
    for (int i = 0; i < 11; i++)
    {
        newDir_Entry.DIR_Name[i] = filename[i];
    }
    newDir_Entry.DIR_Attr = 0x20;
    newDir_Entry.DIR_NTRes = 0;
    newDir_Entry.DIR_FstClusHI = 0;
    newDir_Entry.DIR_FstClusLO = 0;
    newDir_Entry.DIR_FileSize = 0;
    while (NextCluster < 0x0FFFFFF8)
    {
        fseek(ImgFile, ClusterByteOffset(NextCluster), SEEK_SET);
        for (int i = 0; i < (BytesPerCluster / 32); i++)
        {
            int newDirLoc = ftell(ImgFile);
            fread(&Dir_Entry, sizeof(DIR), 1, ImgFile);
            if (Dir_Entry.DIR_Attr == 0x0F)
            {
                continue;
            }
            if (Dir_Entry.DIR_Name[0] == 0 || Dir_Entry.DIR_Name[0] == 0xE5)
            {
                fseek(ImgFile, newDirLoc, SEEK_SET);
                fwrite(&newDir_Entry, sizeof(DIR), 1, ImgFile);
                break;
            }
        }
        NextCluster = GetNextCluster(NextCluster);
    }
}
