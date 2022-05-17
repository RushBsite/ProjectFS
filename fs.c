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
    
    DevCloseDisk();
    return;
}

void SetInodeBytemap(int inodeno)
{
    //Open disk
    DevOpenDisk();

    //Read InodeBlock
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //Set inodeno to '1'
    pBuf[inodeno] = '1';
    DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //close disk
    DevCloseDisk();
    return;
}


void ResetInodeBytemap(int inodeno)
{
    //Open disk
    DevOpenDisk();

    //Read InodeBlock
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);
    
    //Reset inodeno to '0'
    pBuf[inodeno] = '0';
    DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //Open disk
    DevOpenDisk();
    return;
}


void SetBlockBytemap(int blkno)
{
    //Open disk
    DevOpenDisk();

    //Read block bytemap
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Set blkno to '1'
    pBuf[blkno] = '1';
    DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Open disk
    DevOpenDisk();
    return;
}


void ResetBlockBytemap(int blkno)
{
    //Open disk
    DevOpenDisk();

    //Read block bytemap
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Reset blkno to '0'
    pBuf[blkno] = '0';
    DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Open disk
    DevOpenDisk();
    return;
}


void PutInode(int inodeno, Inode* pInode)
{

}


void GetInode(int inodeno, Inode* pInode)
{
    //
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
