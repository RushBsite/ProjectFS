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
    int index = 0;
    //extract path & dir name
    char* buf = (char*)malloc(MAX_NAME_LEN);
    char* paths[500] = {NULL, };

    buf = strtok(name,"/");
    while(buf!=NULL){
        paths[index++] = buf;
        buf = strtok(NULL, "/");
    }

    //Get Free No.
    int targetBlockNum = GetFreeBlockNum();
    int targetInodeNum = GetFreeInodeNum();

    //**---Set root Dir Block---**

    //Get root Inode
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    GetInode(0,pInode);
    int rootBlkNum = pInode->dirBlockPtr[0];

    //Read root block
    DirEntry* pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
    char* pBuf = (char*) pEntry;
    //DevReadBlock(rootBlkNum,pBuf);

    //Set DirEntry in paths blocks

    //1. Check Possible
    int check = 0;
    int targetDirEntryNum = -1;
    int targetdirBlkPtr = -1;
    int nextInodeNum = 0;
    for(int i=0;i<index;i++){
        for(int j=0;j<NUM_OF_DIRECT_BLOCK_PTR;j++){ //all dirptr
            if(pInode->dirBlockPtr[i] == -1){//need new dirblock in prev dir
                //allocate new block
                DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                char* n_pBuf = (char*) n_pEntry;
                int newBlkNum = GetFreeBlockNum();

                for(int i=1;i<NUM_OF_DIRENT_PER_BLK;i++){
                memset(n_pEntry[i].name, INVALID_ENTRY, MAX_NAME_LEN);
                    n_pEntry[i].inodeNum = INVALID_ENTRY;
                }
                //init block
                DevWriteBlock(newBlkNum, n_pBuf);
                
                //update Inode
                pInode->allocBlocks++;
                pInode->size += BLOCK_SIZE;
                pInode->dirBlockPtr[i] = newBlkNum;

                PutInode(nextInodeNum, pInode);

                //set bytemap
                SetBlockBytemap(newBlkNum);    
            
                //update FileSystemInfo
                pFileSysInfo->numFreeBlocks--;
                pFileSysInfo->numAllocBlocks++;
                DevWriteBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);

                free(n_pEntry);
            }
            DevReadBlock(pInode->dirBlockPtr[i], pBuf); 
            targetdirBlkPtr = i;
            for(int k=0; k< NUM_OF_DIRENT_PER_BLK; k++){
                //if name match
                if(i != index-1 && strcmp(paths[i],pEntry[k].name)==0){
                //find next Inode num
                    nextInodeNum = pEntry[k].inodeNum;
                    GetInode(nextInodeNum,pInode);
                    check++;
                    break;
                }
            //if last path(dir name) & duplicate
                else if(i == index-1 && strcmp(paths[i],pEntry[k].name)==0){
                    return -1;
                }
            //if last path(dir name) & empty space
                if(i==index-1 && pEntry[k].inodeNum == INVALID_ENTRY){
                    targetDirEntryNum = k;
                    check++;
                    break;
                }
            }
        }
    }
    if(check != index) return -1;
  
    //2. if Possible : Set DirEntry

    //set prev direntry
    strncpy(pEntry[targetDirEntryNum].name,paths[index-1],strlen(paths[index-1]));
    pEntry[targetDirEntryNum].inodeNum = targetInodeNum;
    DevWriteBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);

    //set new direntry
    DevReadBlock(targetBlockNum,pBuf);

    strncpy(pEntry[0].name,".",1);
    pEntry[0].inodeNum = targetInodeNum;
    strncpy(pEntry[1].name,"..",2);
    pEntry[1].inodeNum = nextInodeNum;

    DevWriteBlock(targetBlockNum,pBuf);

    //**---Set Inode---**
    GetInode(targetInodeNum, pInode);
    pInode->allocBlocks = 1;
    pInode->type =FILE_TYPE_DIR;
    pInode->size =BLOCK_SIZE;
    PutInode(targetInodeNum, pInode);

    //**---Set bytemap---**
    SetBlockBytemap(targetBlockNum);
    SetInodeBytemap(targetInodeNum);

    //**---FileSysInfo handle---**
    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks--;
    pFileSysInfo->numAllocInodes++;
    DevWriteBlock(FILESYS_INFO_BLOCK,(char*)pFileSysInfo);
    
    free(pEntry);
    free(pInode);
    free(buf);
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
    strncpy(pDirEntry[0].name, ".",1);
    pDirEntry[0].inodeNum = 0;

    //set other DirEntry into INVALID_ENTRY
    for(int i=1;i<NUM_OF_DIRENT_PER_BLK;i++){
        memset(pDirEntry[i].name, INVALID_ENTRY, MAX_NAME_LEN);
        pDirEntry[i].inodeNum = INVALID_ENTRY;
    }

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
    for(int i=1;i<NUM_OF_DIRECT_BLOCK_PTR;i++){
        pInode->dirBlockPtr[i] = -1;
    }
    
    //Put target num Inode
    PutInode(targetInodeNum, pInode);

    free(pFileSysInfo);
    free(pBuf);
    free(pInode);
}
void OpenFileSystem(void)
{
    DevOpenDisk();
    pFileSysInfo = (FileSysInfo*)malloc(BLOCK_SIZE);
    char* pBuf = (char*)pFileSysInfo;
    memset(pFileSysInfo, 0, BLOCK_SIZE);
    DevReadBlock(0, pBuf);
}

void CloseFileSystem(void)
{
    free(pFileSysInfo);
    DevCloseDisk();
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
