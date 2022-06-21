#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

FileDescTable* pFileDescTable;
FileSysInfo* pFileSysInfo;

int OpenFile(const char* name, OpenFlag flag)
{

}


int WriteFile(int fileDesc, char* pBuffer, int length)
{

}


int ReadFile(int fileDesc, char* pBuffer, int length)
{

}

int CloseFile(int fileDesc)
{

}

int RemoveFile(char* name)
{

}


int MakeDirectory(char* name)
{

}


int RemoveDirectory(char* name)
{

}


void CreateFileSystem(void)
{
    //file system init
    FileSysInit();

    //**----create root dir---**

    //GetFree No.
    int targetBlockNum = GetFreeBlockNum();
    int targetInodeNum = GetFreeInodeNum();
    
    //typecast block size into DirEntry
    DirEntry* pDirEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
    char* pBuf = (char*) pDirEntry;

    //Init DirEntry[0]
    strcpy(pDirEntry[0].name, ".");
    pDirEntry[0].inodeNum = 0;

    //Save root Dir
    DevWriteBlock(targetBlockNum, pBuf);

    //**---Create FileSysInfo block---**

    //Init FileSysInfo, typecast
    pFileSysInfo = (FileSysInfo*)malloc(BLOCK_SIZE);
    memset(pFileSysInfo, 0, BLOCK_SIZE);
    pBuf = (char*) pFileSysInfo;
    
    int numTotalBlocks = FS_DISK_CAPACITY/BLOCK_SIZE;
    pFileSysInfo->blocks = numTotalBlocks;
    pFileSysInfo->rootInodeNum = 0;
    pFileSysInfo->diskCapacity = FS_DISK_CAPACITY;
    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks = numTotalBlocks - (INODELIST_BLOCK_FIRST+INODELIST_BLOCKS) -1;
    pFileSysInfo->numAllocInodes++;
    pFileSysInfo->blockBytemapBlock = BLOCK_BYTEMAP_BLOCK_NUM;
    pFileSysInfo->inodeBytemapBlock = INODE_BYTEMAP_BLOCK_NUM;
    pFileSysInfo->inodeListBlock = INODELIST_BLOCK_FIRST;
    pFileSysInfo->dataRegionBlock = INODELIST_BLOCK_FIRST+INODELIST_BLOCKS;

    DevWriteBlock(FILESYS_INFO_BLOCK,pBuf);
    
    //Set Block bytemap
    SetBlockBytemap(targetBlockNum);
    SetInodeBytemap(targetInodeNum);

    //Get target num Inode
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    GetInode(targetInodeNum, pInode);

    //Init root dir Inode
    pInode->allocBlocks = 1;
    pInode->type = FILE_TYPE_DIR;
    pInode->size = BLOCK_SIZE;
    pInode->dirBlockPtr[0] = INODELIST_BLOCK_FIRST+INODELIST_BLOCKS;
    
    //Put target num Inode
    PutInode(targetInodeNum, pInode);

    free(pFileSysInfo);
    free(pBuf);
    free(pInode);
}
void OpenFileSystem(void)
{

}

void CloseFileSystem(void)
{

}

Directory* OpenDirectory(char* name)
{

}


FileInfo* ReadDirectory(Directory* pDir)
{

}

int CloseDirectory(Directory* pDir)
{

}
