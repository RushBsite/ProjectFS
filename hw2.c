#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

FileDescTable* pFileDescTable;
FileSysInfo* _pFileSysInfo;
int offset = 0;

int OpenFile(const char* name, OpenFlag flag)
{
    return 0;
}


int WriteFile(int fileDesc, char* pBuffer, int length)
{
    return 0;
}


int ReadFile(int fileDesc, char* pBuffer, int length)
{
    return 0;
}

int CloseFile(int fileDesc)
{
    return 0;
}

int RemoveFile(char* name)
{
    return 0;
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
            int find = 0;
            if(pInode->dirBlockPtr[j] == -1){//need new dirblock in prev dir
                //allocate new block
                DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                char* n_pBuf = (char*) n_pEntry;
                int newBlkNum = GetFreeBlockNum();

                for(int m=0;m<NUM_OF_DIRENT_PER_BLK;m++){
                memset(n_pEntry[m].name, 0, MAX_NAME_LEN);
                    n_pEntry[m].inodeNum = INVALID_ENTRY;
                }
                //init block
                DevWriteBlock(newBlkNum, n_pBuf);
                
                //update Inode
                pInode->allocBlocks++;
                pInode->size += BLOCK_SIZE;
                pInode->dirBlockPtr[j] = newBlkNum;

                PutInode(nextInodeNum, pInode);

                //set bytemap
                SetBlockBytemap(newBlkNum);    
            
                //update FileSystemInfo
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                _pFileSysInfo->numFreeBlocks--;
                _pFileSysInfo->numAllocBlocks++;
                DevWriteBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

                free(n_pEntry);
            }
            DevReadBlock(pInode->dirBlockPtr[j], pBuf); 
            targetdirBlkPtr = j;
            for(int k=0; k< NUM_OF_DIRENT_PER_BLK; k++){
                //if name match
                if(i != index-1 && strcmp(paths[i],pEntry[k].name)==0){
                //find next Inode num
                    nextInodeNum = pEntry[k].inodeNum;
                    GetInode(nextInodeNum,pInode);
                    check++;
                    find = 1;
                    break;
                }
            //if last path(dir name) & duplicate
                else if(i == index-1 && strcmp(paths[i],pEntry[k].name)==0){
                    free(pEntry);
                    free(pInode);
                    free(buf);
                    return -1;
                }
            //if last path(dir name) & empty space
                if(i==index-1 && pEntry[k].inodeNum == INVALID_ENTRY){
                    targetDirEntryNum = k;
                    check++;
                    find = 1;
                    break;
                }
            }

            if(find) break;
        }
    }
    if(check != index) {
        free(pEntry);
        free(pInode);
        free(buf);
        return -1;
    }
  
    //2. if Possible : Set DirEntry

     //Get Free No.
    int targetBlockNum = GetFreeBlockNum();
    int targetInodeNum = GetFreeInodeNum();

    //set prev direntry
    DevReadBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);
    strncpy(pEntry[targetDirEntryNum].name,paths[index-1],strlen(paths[index-1])+1);
    pEntry[targetDirEntryNum].inodeNum = targetInodeNum;
    DevWriteBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);

    //set new direntry
    DevReadBlock(targetBlockNum,pBuf);

    strncpy(pEntry[0].name,".",2);
    pEntry[0].inodeNum = targetInodeNum;
    strncpy(pEntry[1].name,"..",3);
    pEntry[1].inodeNum = nextInodeNum;
    
    for(int i=2;i<NUM_OF_DIRENT_PER_BLK;i++){
        memset(pEntry[i].name, 0, MAX_NAME_LEN);
        pEntry[i].inodeNum = INVALID_ENTRY;
    }

    DevWriteBlock(targetBlockNum,pBuf);

    //**---Set Inode---**
    GetInode(targetInodeNum, pInode);
    pInode->allocBlocks = 1;
    pInode->type =FILE_TYPE_DIR;
    pInode->size =BLOCK_SIZE;
    pInode->dirBlockPtr[0] = targetBlockNum;
    for(int i=1;i<NUM_OF_DIRECT_BLOCK_PTR;i++){
        pInode->dirBlockPtr[i] = -1;
    }
    PutInode(targetInodeNum, pInode);

    //**---Set bytemap---**
    SetBlockBytemap(targetBlockNum);
    SetInodeBytemap(targetInodeNum);

    //**---FileSysInfo handle---**
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
    _pFileSysInfo->numAllocBlocks++;
    _pFileSysInfo->numFreeBlocks--;
    _pFileSysInfo->numAllocInodes++;
    DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
    
    free(pEntry);
    free(pInode);
    free(buf);

    return 0;
}


int RemoveDirectory(char* name)
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

    //Get root Inode
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    GetInode(0,pInode);
    int rootBlkNum = pInode->dirBlockPtr[0];

    //Read root block
    DirEntry* pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
    char* pBuf = (char*) pEntry;

    //1. Check Possible
    int check = 0;
    int nextInodeNum = 0;
    int prevInodeNum = 0;
    int targetdirBlkPtr = -1;
    int targetIndex = -1;
    for(int i=0;i<index;i++){
        for(int j=0;j<NUM_OF_DIRECT_BLOCK_PTR;j++){ //all dirptr
            int find = 0;
           if(pInode->dirBlockPtr[j] == -1){
                free(pEntry);
                free(pInode);
                free(buf);
                return -1;
           }
           targetdirBlkPtr = j;
           DevReadBlock(pInode->dirBlockPtr[j], pBuf); 
            for(int k=0;k<NUM_OF_DIRENT_PER_BLK;k++){
                //if name match
                if(strcmp(paths[i],pEntry[k].name)==0){
                //find next Inode num
                    prevInodeNum = nextInodeNum;
                    targetIndex = k;
                    nextInodeNum = pEntry[k].inodeNum;
                    GetInode(nextInodeNum,pInode);
                    find = 1;
                    check++;
                    break;
                }
                else if(pEntry[k].inodeNum == INVALID_ENTRY){
                    free(pEntry);
                    free(pInode);
                    free(buf);
                    return -1;
                }
            }
            if(find) break;
        }
    }

    if(check != index) {
        free(pEntry);
        free(pInode);
        free(buf);
        return -1;
    }

    //2. Check Dir is Empty
    DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry));
    GetDirEntry(pInode->dirBlockPtr[0],2,n_pEntry);
    if(n_pEntry->inodeNum != INVALID_ENTRY){
        free(n_pEntry);
        free(pEntry);
        free(pInode);
        free(buf);
        return -1;
    }
    //Reset Cur Bytemap
    ResetInodeBytemap(nextInodeNum);
    ResetBlockBytemap(pInode->dirBlockPtr[0]);

    //Delete cur Dir
    pBuf = (char*) malloc(BLOCK_SIZE*(sizeof(char)));
    memset(pBuf, 0, BLOCK_SIZE*sizeof(char));
    DevWriteBlock(pInode->dirBlockPtr[0], pBuf);

    //Reset Cur Inode
    memset(pInode, 0, sizeof(Inode));
    PutInode(nextInodeNum, pInode);


    //Re set FileSysInfo
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
    _pFileSysInfo->numFreeBlocks++;
    _pFileSysInfo->numAllocInodes--;
    _pFileSysInfo->numAllocBlocks--;
    DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);


    //Delete cur dir in prev dir
    GetInode(prevInodeNum, pInode);
    RemoveDirEntry(pInode->dirBlockPtr[targetdirBlkPtr], targetIndex);

    //re construct prevdir entrys

    for(int i=targetdirBlkPtr;i<NUM_OF_DIRECT_BLOCK_PTR;i++){
        if(pInode->dirBlockPtr[i]==INVALID_ENTRY) break;
        int finish = 0;
        if(targetIndex == NUM_OF_DIRENT_PER_BLK -1){
            if(i!= NUM_OF_DIRECT_BLOCK_PTR-1 && pInode->dirBlockPtr[i+1] != INVALID_ENTRY){
                GetDirEntry(pInode->dirBlockPtr[i+1],0,n_pEntry);
                PutDirEntry(pInode->dirBlockPtr[i],targetIndex,n_pEntry);
                RemoveDirEntry(pInode->dirBlockPtr[i+1],0);
                targetIndex = 0;
            }
        }

        for(int j=targetIndex+1;j<NUM_OF_DIRENT_PER_BLK;j++){
            GetDirEntry(pInode->dirBlockPtr[i], j, n_pEntry);
            if(n_pEntry->inodeNum == INVALID_ENTRY){ //finish
                if(j == 1){ // reset prev Inode
                    pInode->allocBlocks--;
                    pInode->dirBlockPtr[i] = -1;
                    pInode->size -= BLOCK_SIZE;
                    PutInode(prevInodeNum,pInode);

                    //reset FileSysInfo
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    _pFileSysInfo->numFreeBlocks++;
                    _pFileSysInfo->numAllocBlocks--;
                    DevWriteBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    finish = 1;
                }
                break;
            }
            PutDirEntry(pInode->dirBlockPtr[i],j-1,n_pEntry);
            RemoveDirEntry(pInode->dirBlockPtr[i],j);
        }
        if(finish) break;
        targetIndex = NUM_OF_DIRENT_PER_BLK-1;
    }

    free(n_pEntry);
    free(pEntry);
    free(pInode);
    free(buf);
    return 0;
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
    strncpy(pDirEntry[0].name, ".",2);
    pDirEntry[0].inodeNum = 0;

    //set other DirEntry into INVALID_ENTRY
    for(int i=1;i<NUM_OF_DIRENT_PER_BLK;i++){
        memset(pDirEntry[i].name, 0, MAX_NAME_LEN);
        pDirEntry[i].inodeNum = INVALID_ENTRY;
    }

    //Save root Dir
    DevWriteBlock(targetBlockNum, pBuf);

    //**---Create FileSysInfo block---**

    //Init FileSysInfo, typecast
    _pFileSysInfo = (FileSysInfo*)malloc(BLOCK_SIZE);
    memset(_pFileSysInfo, 0, BLOCK_SIZE);
    pBuf = (char*) _pFileSysInfo;
    
    int numTotalBlocks = FS_DISK_CAPACITY/BLOCK_SIZE;
    _pFileSysInfo->blocks = numTotalBlocks;
    _pFileSysInfo->rootInodeNum = 0;
    _pFileSysInfo->diskCapacity = FS_DISK_CAPACITY;
    _pFileSysInfo->numAllocBlocks = 1;
    _pFileSysInfo->numFreeBlocks = numTotalBlocks - (INODELIST_BLOCK_FIRST+INODELIST_BLOCKS) -1;
    _pFileSysInfo->numAllocInodes = 1;
    _pFileSysInfo->blockBytemapBlock = BLOCK_BYTEMAP_BLOCK_NUM;
    _pFileSysInfo->inodeBytemapBlock = INODE_BYTEMAP_BLOCK_NUM;
    _pFileSysInfo->inodeListBlock = INODELIST_BLOCK_FIRST;
    _pFileSysInfo->dataRegionBlock = INODELIST_BLOCK_FIRST+INODELIST_BLOCKS;

    DevWriteBlock(FILESYS_INFO_BLOCK,pBuf);
    DevReadBlock(FILESYS_INFO_BLOCK, pBuf);
    
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
    _pFileSysInfo = (FileSysInfo*)malloc(BLOCK_SIZE);
    char* pBuf = (char*)_pFileSysInfo;
    memset(_pFileSysInfo, 0, BLOCK_SIZE);
    DevReadBlock(0, pBuf);
}

void CloseFileSystem(void)
{
    free(_pFileSysInfo);
    DevCloseDisk();
}

Directory* OpenDirectory(char* name)
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

    //Get root Inode
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    GetInode(0,pInode);
    int rootBlkNum = pInode->dirBlockPtr[0];

    //Read root block
    DirEntry* pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
    char* pBuf = (char*) pEntry;
    DevReadBlock(rootBlkNum,pBuf);

    int check = 0;
    int nextInodeNum = 0;
    int prevInodeNum = 0;
    int targetdirBlkPtr = -1;
    int targetIndex = -1;

    for(int i=0;i<index;i++){
        for(int j=0;j<NUM_OF_DIRECT_BLOCK_PTR;j++){ //all dirptr
            int find = 0;
           if(pInode->dirBlockPtr[j] == -1){
                free(pEntry);
                free(pInode);
                free(buf);
                return NULL;
           }
           targetdirBlkPtr = j;
           DevReadBlock(pInode->dirBlockPtr[j], pBuf); 
            for(int k=0;k<NUM_OF_DIRENT_PER_BLK;k++){
                //if name match
                if(strcmp(paths[i],pEntry[k].name)==0){
                //find next Inode num
                    prevInodeNum = nextInodeNum;
                    targetIndex = k;
                    nextInodeNum = pEntry[k].inodeNum;
                    GetInode(nextInodeNum,pInode);
                    find = 1;
                    check++;
                    break;
                }
                else if(pEntry[k].inodeNum == INVALID_ENTRY){
                    free(pEntry);
                    free(pInode);
                    free(buf);
                    return NULL;
                }
            }
            if(find) break;
        }
    }
    if(check != index) {
        free(pEntry);
        free(pInode);
        free(buf);
        return NULL;
    }

    Directory* dir = (Directory*)malloc(sizeof(Directory));
    memset(dir,0,sizeof(Directory));
    dir->inodeNum = nextInodeNum;

    return dir;
}


FileInfo* ReadDirectory(Directory* pDir)
{
    
    //2. return FileInfo
    int _inodeNum = pDir->inodeNum;
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    DirEntry* pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
    FileInfo* pFileInfo = (FileInfo*)malloc(sizeof(FileInfo));
    memset(pFileInfo,0,sizeof(FileInfo));
    GetInode(_inodeNum,pInode);

    //search
    int curDirPtr = offset/ NUM_OF_DIRENT_PER_BLK;
    int curEntry = offset% NUM_OF_DIRENT_PER_BLK;

    if(curDirPtr >=NUM_OF_DIRECT_BLOCK_PTR || curEntry>=NUM_OF_DIRENT_PER_BLK){
        offset = 0;
        return NULL;
    }

    if(pInode->dirBlockPtr[curDirPtr] == -1){
        offset = 0;
        return NULL;
    }

    DevReadBlock(pInode->dirBlockPtr[curDirPtr], (char*)pEntry);

    if(pEntry[curEntry].inodeNum == INVALID_ENTRY){
        offset = 0;
        return NULL;
    }

    GetInode(pEntry[curEntry].inodeNum,pInode);

    strncpy(pFileInfo->name,pEntry[curEntry].name,strlen(pEntry[curEntry].name));
    pFileInfo->filetype = pInode->type;
    pFileInfo->inodeNum = pEntry[curEntry].inodeNum;
    pFileInfo->numBlocks = pInode->allocBlocks;
    pFileInfo->size = pInode->size;
    
    //set next inodenum
    offset++;

    return pFileInfo;
}

int CloseDirectory(Directory* pDir)
{
    free(pDir);
    return 0;
}
