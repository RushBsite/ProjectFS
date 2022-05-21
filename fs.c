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
    int NUM_OF_INODE_IN_BLOCK = BLOCK_SIZE/sizeof(Inode);
    int block_num = inodeno/NUM_OF_INODE_IN_BLOCK + INODELIST_BLOCK_FIRST; //physical block number
    int inodeIdx = inodeno%NUM_OF_INODE_IN_BLOCK; //physical inode block index

    //Open disk
    DevOpenDisk();

    //type conversion & Read Block number block
    Inode* ptr = (Inode*)malloc(sizeof(Inode)*NUM_OF_INODE_IN_BLOCK);
    char* pBuf = (char*)ptr;
    DevReadBlock(block_num, pBuf);

    //copy & write
    memcpy(&ptr[inodeIdx],pInode,sizeof(Inode));
    DevWriteBlock(block_num,pBuf);

    //Close disk
    DevCloseDisk();
    free(pBuf);
    return; 
}


void GetInode(int inodeno, Inode* pInode)
{
    //block number where inode stored
    int NUM_OF_INODE_IN_BLOCK = BLOCK_SIZE/sizeof(Inode);
    int block_num = inodeno/NUM_OF_INODE_IN_BLOCK + INODELIST_BLOCK_FIRST; //physical block number
    int inodeIdx = inodeno%NUM_OF_INODE_IN_BLOCK; //physical inode block index

    //Open disk
    DevOpenDisk();

    //type conversion & Read Block number block
    Inode* ptr = (Inode*)malloc(sizeof(Inode)*NUM_OF_INODE_IN_BLOCK);
    char* pBuf = (char*)ptr;
    DevReadBlock(block_num, pBuf); 
    
    //save
    memcpy(pInode, &ptr[inodeIdx],sizeof(Inode));

    //Close disk
    DevCloseDisk();
    free(pBuf);
    return;       
}


int GetFreeInodeNum(void)
{
    //Open disk
    DevOpenDisk();

    //Read Block number block
    char *pBuf = (char*)malloc(BLOCK_SIZE*sizeof(char));
    DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuf);

    //Close disk
    DevCloseDisk();

    //First fit search 
    for(int i=0;i<BLOCK_SIZE;i++){
        if(!pBuf[i]) {free(pBuf); return i;}
    }

    free(pBuf);    
    return -1;
}


int GetFreeBlockNum(void)
{
    //Open disk
    DevOpenDisk();

    //Read Block number block
    char *pBuf = (char*)malloc(BLOCK_SIZE*sizeof(char));
    DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuf);

    //Close disk
    DevCloseDisk();

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

    //Open disk
    DevOpenDisk();

    //type conversion
    int *ptr = (int*)malloc(NUM_OF_ENTRY_IN_BLOCK*sizeof(int));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //copy & write
    memcpy(&ptr[index],&number,sizeof(int));
    DevWriteBlock(blkno,pBuf);

    //Close disk
    //DevCloseDisk();
    free(pBuf);
    return;  


}

int GetIndirectBlockEntry(int blkno, int index)
{
    int NUM_OF_ENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(int);

    //Open disk
    DevOpenDisk();

    //type conversion
    int *ptr = (int*)malloc(NUM_OF_ENTRY_IN_BLOCK*sizeof(int));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //copy
    int temp = ptr[index];

    //Close disk
    //DevCloseDisk();
    free(ptr);
    //printf("entry no: %d, value: %d\n", index, temp);
    return temp;  
}

void RemoveIndirectBlockEntry(int blkno, int index)
{
    int NUM_OF_ENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(int);

    //Open disk
    DevOpenDisk();

    //type conversion
    int *ptr = (int*)malloc(NUM_OF_ENTRY_IN_BLOCK*sizeof(int));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //set Invaild
    ptr[index] = INVALID_ENTRY;

    //write    
    DevWriteBlock(blkno,pBuf);

    //Close disk
    //DevCloseDisk();
    free(pBuf);
    return; 
}

void PutDirEntry(int blkno, int index, DirEntry* pEntry)
{
    int NUM_OF_DIRENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(DirEntry);

    //Open disk
    DevOpenDisk();

    //type conversion
    DirEntry *ptr = (DirEntry*)malloc(NUM_OF_DIRENTRY_IN_BLOCK*sizeof(DirEntry));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //copy & write
    memcpy(pBuf+(index*32),pEntry,sizeof(DirEntry));
    DevWriteBlock(blkno,pBuf);

    //Close disk
    //DevCloseDisk();
    free(ptr);
    return;
}

int GetDirEntry(int blkno, int index, DirEntry* pEntry)
{
    int NUM_OF_DIRENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(DirEntry);

    //Open disk
    DevOpenDisk();

    //type conversion
    DirEntry *ptr = (DirEntry*)malloc(NUM_OF_DIRENTRY_IN_BLOCK*sizeof(DirEntry));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);
    
    //save
    memcpy(pEntry, &ptr[index], sizeof(DirEntry));
    
    //Close disk
    //DevCloseDisk();
    free(pBuf);

    //Invalid check
    if(pEntry->inodeNum == INVALID_ENTRY) return INVALID_ENTRY;
    else return 1;  
}

void RemoveDirEntry(int blkno, int index)
{
    int NUM_OF_DIRENTRY_IN_BLOCK = BLOCK_SIZE/sizeof(DirEntry);
    
    //Open disk
    DevOpenDisk();

    //type conversion
    DirEntry *ptr = (DirEntry*)malloc(NUM_OF_DIRENTRY_IN_BLOCK*sizeof(DirEntry));
    char *pBuf = (char*)ptr;

    //Read Block number block
    DevReadBlock(blkno, pBuf);

    //set Invaild val
    ptr[index].inodeNum = INVALID_ENTRY;

    //write
    DevWriteBlock(blkno,pBuf);

    //Close disk
    //DevCloseDisk();
    free(pBuf);
    return;
}
