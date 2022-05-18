#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fs.h"

#define NUM_OF_INODE_IN_BLOCK   (16)
#define NUM_OF_BLOCK_IN_INODELIST   (32)

void FileSysInit(void)
{
    DevCreateDisk();
    //char *pBuf = (char*) calloc(BLOCK_SIZE,sizeof(char)); //block size allocate, init all by '0'
    //memory with 0 saved in block 0to6
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    memset(pBuf, 0, BLOCK_SIZE*sizeof(char));

    for(int i=0;i<=6;i++)
        DevWriteBlock(i,pBuf);
    
    DevCloseDisk();
    free(pBuf);
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
    pBuf[inodeno] = 1;
    DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //close disk
    DevCloseDisk();
    free(pBuf);
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
    pBuf[inodeno] = 0;
    DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //Close disk
    DevCloseDisk();
    free(pBuf);
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
    pBuf[blkno] = (char)1;
    DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Close disk
    DevCloseDisk();
    free(pBuf);
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
    pBuf[blkno] = 0;
    DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Close disk
    DevCloseDisk();
    free(pBuf);
    return;
}


void PutInode(int inodeno, Inode* pInode)
{
    //block number where inode stored
    int block_num = inodeno/NUM_OF_INODE_IN_BLOCK + INODELIST_BLOCK_FIRST; //physical block number
    int inodeIdx = inodeno%NUM_OF_INODE_IN_BLOCK; //physical inode block index

    //Open disk
    DevOpenDisk();

    //type conversion & Read Block number block
    Inode* ptr = (Inode*)malloc(sizeof(Inode)*NUM_OF_INODE_IN_BLOCK);
    char* pBuf = (char*)ptr;
    DevReadBlock(block_num, pBuf);

    //copy & write
    memcpy(&pBuf[NUM_OF_BLOCK_IN_INODELIST*inodeIdx],pInode,sizeof(Inode));
    DevWriteBlock(block_num,pBuf);

    //Close disk
    DevCloseDisk();
    free(pBuf);
    free(ptr);
    return; 
}


void GetInode(int inodeno, Inode* pInode)
{
    //block number where inode stored
    int block_num = inodeno/NUM_OF_INODE_IN_BLOCK + INODELIST_BLOCK_FIRST; //physical block number
    int inodeIdx = inodeno%NUM_OF_INODE_IN_BLOCK; //physical inode block index

    //Open disk
    DevOpenDisk();

    //Read Block number block
    char *pBuf = (char*)malloc(BLOCK_SIZE*sizeof(char));
    DevReadBlock(block_num, pBuf);
    
    //type conversion
    Inode* ptr = (Inode*)&pBuf[NUM_OF_BLOCK_IN_INODELIST*inodeIdx];

    //save
    memcpy(pInode, ptr,sizeof(Inode));

    //Close disk
    DevCloseDisk();
    free(pBuf);
    free(ptr);
    return;       
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
