#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs.h"

void FileSysInit(void)
{
    DevCreateDisk();
    //char *pBuf = (char*) calloc(BLOCK_SIZE,sizeof(char)); //block size allocate, init all by '0'
    //memory with 0 saved in block 0to6
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    memset(pBuf, '0', BLOCK_SIZE*sizeof(char));

    for(int i=0;i<=6;i++)
        DevWriteBlock(i,pBuf);
    return;
}

void SetInodeBytemap(int inodeno)
{

}


void ResetInodeBytemap(int inodeno)
{

}


void SetBlockBytemap(int blkno)
{

}


void ResetBlockBytemap(int blkno)
{

}


void PutInode(int inodeno, Inode* pInode)
{

}


void GetInode(int inodeno, Inode* pInode)
{

}


int GetFreeInodeNum(void)
{

}


int GetFreeBlockNum(void)
{

}

void PutIndirectBlockEntry(int blkno, int index, int number)
{

}

int GetIndirectBlockEntry(int blkno, int index)
{

}

void RemoveIndirectBlockEntry(int blkno, int index)
{
    
}

void PutDirEntry(int blkno, int index, DirEntry* pEntry)
{

}

int GetDirEntry(int blkno, int index, DirEntry* pEntry)
{

}

void RemoveDirEntry(int blkno, int index)
{

}
