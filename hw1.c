#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"

void FileSysInit(void)
{
    DevCreateDisk();
    //char *pBuf = (char*) calloc(BLOCK_SIZE,sizeof(char)); //block size allocate, init all by '0'
    //memory with 0 saved in block 0to6
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    memset(pBuf, 0, BLOCK_SIZE*sizeof(char));

    DevOpenDisk();

    for(int i=0;i<=6;i++)
        DevWriteBlock(i,pBuf);
    
    free(pBuf);
    return;
}

void SetInodeBytemap(int inodeno)
{
    //Read InodeBlock
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //Set inodeno to '1'
    pBuf[inodeno] = 1;
    DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    free(pBuf);
    return;
}


void ResetInodeBytemap(int inodeno)
{
    //Read InodeBlock
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);
    
    //Reset inodeno to '0'
    pBuf[inodeno] = 0;
    DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    free(pBuf);
    return;
}


void SetBlockBytemap(int blkno)
{
    //Read block bytemap
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Set blkno to '1'
    pBuf[blkno] = (char)1;
    DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    free(pBuf);
    return;
}


void ResetBlockBytemap(int blkno)
{
    //Read block bytemap
    char *pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Reset blkno to '0'
    pBuf[blkno] = 0;
    DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    free(pBuf);
    return;
}


void PutInode(int inodeno, Inode* pInode)
{
    //block number where inode stored
    int NUM_OF_INODE_IN_BLOCK = BLOCK_SIZE/sizeof(Inode);
    int block_num = inodeno/NUM_OF_INODE_IN_BLOCK + INODELIST_BLOCK_FIRST; //physical block number
    int inodeIdx = inodeno%NUM_OF_INODE_IN_BLOCK; //physical inode block index

    //type conversion & Read Block number block
    Inode* ptr = (Inode*)malloc(sizeof(Inode)*NUM_OF_INODE_IN_BLOCK);
    char* pBuf = (char*)ptr;
    DevReadBlock(block_num, pBuf);

    //copy & write
    memcpy(&ptr[inodeIdx],pInode,sizeof(Inode));
    DevWriteBlock(block_num,pBuf);

    free(pBuf);
    return; 
}


void GetInode(int inodeno, Inode* pInode)
{
    //block number where inode stored
    int NUM_OF_INODE_IN_BLOCK = BLOCK_SIZE/sizeof(Inode);
    int block_num = inodeno/NUM_OF_INODE_IN_BLOCK + INODELIST_BLOCK_FIRST; //physical block number
    int inodeIdx = inodeno%NUM_OF_INODE_IN_BLOCK; //physical inode block index

    //type conversion & Read Block number block
    Inode* ptr = (Inode*)malloc(sizeof(Inode)*NUM_OF_INODE_IN_BLOCK);
    char* pBuf = (char*)ptr;
    DevReadBlock(block_num, pBuf); 
    
    //save
    memcpy(pInode, &ptr[inodeIdx],sizeof(Inode));

    free(pBuf);
    return;       
}


int GetFreeInodeNum(void)
{
    //Read Block number block
    char *pBuf = (char*)malloc(BLOCK_SIZE*sizeof(char));
    DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //First fit search 
    for(int i=0;i<BLOCK_SIZE;i++){
        if(!pBuf[i]) {free(pBuf); return i;}
    }

    free(pBuf);    
    return -1;
}


int GetFreeBlockNum(void)
{

    //Read Block number block
    char *pBuf = (char*)malloc(BLOCK_SIZE*sizeof(char));
    DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //First fit search 
    for(int i=0;i<BLOCK_SIZE;i++){
        if(!pBuf[i]) {free(pBuf); return i;}
    }

    free(pBuf);    
    return -1;
}

void PutIndirectBlockEntry(int blkno, int index, int number)
{
    int NUM_OF_ENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(int);

    //type conversion
    int *ptr = (int*)malloc(NUM_OF_ENTRY_IN_BLOCK*sizeof(int));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //copy & write
    memcpy(&ptr[index],&number,sizeof(int));
    DevWriteBlock(blkno,pBuf);

    free(pBuf);
    return;  


}

int GetIndirectBlockEntry(int blkno, int index)
{
    int NUM_OF_ENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(int);

    //type conversion
    int *ptr = (int*)malloc(NUM_OF_ENTRY_IN_BLOCK*sizeof(int));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //copy
    int temp = ptr[index];

    free(ptr);
    //printf("entry no: %d, value: %d\n", index, temp);
    return temp;  
}

void RemoveIndirectBlockEntry(int blkno, int index)
{
    int NUM_OF_ENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(int);

    //type conversion
    int *ptr = (int*)malloc(NUM_OF_ENTRY_IN_BLOCK*sizeof(int));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //set Invaild
    ptr[index] = INVALID_ENTRY;

    //write    
    DevWriteBlock(blkno,pBuf);

    free(pBuf);
    return; 
}

void PutDirEntry(int blkno, int index, DirEntry* pEntry)
{
    int NUM_OF_DIRENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(DirEntry);

    //type conversion
    DirEntry *ptr = (DirEntry*)malloc(NUM_OF_DIRENTRY_IN_BLOCK*sizeof(DirEntry));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //copy & write
    memcpy(&ptr[index],pEntry,sizeof(DirEntry));
    DevWriteBlock(blkno,pBuf);

    free(ptr);
    return;
}

int GetDirEntry(int blkno, int index, DirEntry* pEntry)
{
    int NUM_OF_DIRENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(DirEntry);

    //type conversion
    DirEntry *ptr = (DirEntry*)malloc(NUM_OF_DIRENTRY_IN_BLOCK*sizeof(DirEntry));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);
    
    //save
    memcpy(pEntry, &ptr[index], sizeof(DirEntry));
    
    free(pBuf);

    //Invalid check
    if(pEntry->inodeNum == INVALID_ENTRY) return INVALID_ENTRY;
    else return 1;  
}

void RemoveDirEntry(int blkno, int index)
{
    int NUM_OF_DIRENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(DirEntry);

    //type conversion
    DirEntry *ptr = (DirEntry*)malloc(NUM_OF_DIRENTRY_IN_BLOCK*sizeof(DirEntry));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //set Invaild val
    ptr[index].inodeNum = INVALID_ENTRY;

    //write
    DevWriteBlock(blkno,pBuf);

    free(pBuf);
    return;
}
