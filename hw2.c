#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

FileDescTable* _pFileDescTable;
FileTable* _pFileTable;
FileSysInfo* _pFileSysInfo;
int offset = 0;

int OpenFile(const char* name, OpenFlag flag)
{
    int index = 0;
    //extract path & file name
    char* buf = (char*)malloc(MAX_NAME_LEN);
    char* paths[500] = {NULL, };

    strcpy(buf, name);
    buf = strtok(buf,"/");
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


    if(flag == OPEN_FLAG_CREATE){
        int exist = 0;

        //1.check exist
        
            int check = 0;
            int targetDirEntryNum = -1;
            int targetdirBlkPtr = -1;
            int targetIndirBlkPtr = -1;
            int nextInodeNum = 0;

            for(int i=0;i<index;i++){
                //search in direct block
                for(int j=0;j<NUM_OF_DIRECT_BLOCK_PTR;j++){ //all dirptr
                    //allocate new block
                    if(pInode->dirBlockPtr[j] == INVALID_ENTRY){
                        
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
                    int find = 0;
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
                        //if last path(file name) & duplicate
                        else if(i == index-1 && strcmp(paths[i],pEntry[k].name)==0){
                                targetDirEntryNum = k;
                                check++;
                                exist = 1;
                                find = 1;
                                break;
                        }
                        //if last path(file name) & empty space
                        if(i==index-1 && pEntry[k].inodeNum == INVALID_ENTRY){
                            targetDirEntryNum = k;
                            check++;
                            find = 1;
                            break;
                        }
                    }
                    if(find) break;
                }
                //search in indirect block
                if(check < i+1 && targetdirBlkPtr > 2) {
                    targetdirBlkPtr = -1;

                    if(pInode->indirectBlockPtr==-1){
                        //allocate index block
                        int idxBlkNum = GetFreeBlockNum();
                        pInode->indirectBlockPtr = idxBlkNum;
                        PutInode(nextInodeNum,pInode);
                    
                        int* pIdx = (int*)malloc(sizeof(int)*128);
                        DevReadBlock(idxBlkNum,(char*)pIdx);
                        for(int j=0;j<128;j++){
                            pIdx[j] = INVALID_ENTRY;
                        }
                        DevWriteBlock(idxBlkNum,(char*)pIdx);

                        //set bytemap
                        SetBlockBytemap(idxBlkNum);

                        //**---FileSysInfo handle---**
                        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                        _pFileSysInfo->numFreeBlocks--;
                        DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                        free(pIdx);
                    }
                     
                    //serach in idxblcks
                    for(int j=0;j<128;j++){
                        int find = 0;  
                        if(GetIndirectBlockEntry(pInode->indirectBlockPtr, j) == INVALID_ENTRY){
                            //allocate new block
                            DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                            char* n_pBuf = (char*) n_pEntry;
                            int nBlkNum = GetFreeBlockNum();

                            for(int m=0;m<NUM_OF_DIRENT_PER_BLK;m++){
                            memset(n_pEntry[m].name, 0, MAX_NAME_LEN);
                                n_pEntry[m].inodeNum = INVALID_ENTRY;
                            }
                            //init block
                            DevWriteBlock(nBlkNum, n_pBuf);
                            
                            //update Inode
                            pInode->allocBlocks++;
                            pInode->size += BLOCK_SIZE;
                            PutInode(nextInodeNum, pInode);

                            //updateindirect block
                            PutIndirectBlockEntry(pInode->indirectBlockPtr, j, nBlkNum);
                            

                            //set bytemap
                            SetBlockBytemap(nBlkNum);    
                        
                            //update FileSystemInfo
                            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                            _pFileSysInfo->numFreeBlocks--;
                            _pFileSysInfo->numAllocBlocks++;
                            DevWriteBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

                            free(n_pEntry);
                        }
                        targetIndirBlkPtr = j;
                        DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,j), pBuf);
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
                                targetDirEntryNum = k;
                                check++;
                                exist = 1;
                                find = 1;
                                break;
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

            }
            if(check != index) {
                free(pEntry);
                free(pInode);
                free(buf);
                return -1;
            }
        

        //2.if exist , open
        if(exist == 1)
        {
            int findex = -1;
            int descInx = -1;

            for(int i=0; i<MAX_FILE_NUM; i++){
                if(_pFileTable->pFile[i].bUsed == 0){
                    _pFileTable->pFile[i].bUsed = 1;
                    _pFileTable->pFile[i].fileOffset = 0;
                    _pFileTable->pFile[i].inodeNum = pEntry[targetDirEntryNum].inodeNum;
                    findex = i;
                    break;
                }
            }
            _pFileTable->numUsedFile++;
            if(findex == -1) return -1;
            for(int i=0;i<DESC_ENTRY_NUM;i++){
                if(_pFileDescTable->pEntry[i].bUsed == 0 && _pFileDescTable->pEntry[i].fileTableIndex == findex){
                    _pFileDescTable->pEntry[i].bUsed = 1;
                    descInx = i;
                    break;
                }
            }
            _pFileDescTable->numUsedDescEntry++;
            return descInx;
        }
        //3. if not , create
        if(exist ==0)
        {
             //Get Free No.
            int targetInodeNum = GetFreeInodeNum();

            //set prev direntry
            if(targetdirBlkPtr !=-1){
                DevReadBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);
                strncpy(pEntry[targetDirEntryNum].name,paths[index-1],strlen(paths[index-1])+1);
                pEntry[targetDirEntryNum].inodeNum = targetInodeNum;
                DevWriteBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);
            }
            else if(targetIndirBlkPtr != -1 && targetdirBlkPtr == -1){
                DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,targetIndirBlkPtr),pBuf);
                strncpy(pEntry[targetDirEntryNum].name,paths[index-1],strlen(paths[index-1])+1);
                pEntry[targetDirEntryNum].inodeNum = targetInodeNum;
                DevWriteBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,targetIndirBlkPtr),pBuf);
            }
        
            //**---Set Inode---**
            GetInode(targetInodeNum, pInode);
            pInode->allocBlocks = 0;
            pInode->type =FILE_TYPE_FILE;
            pInode->size = 0;
            for(int i=0;i<NUM_OF_DIRECT_BLOCK_PTR;i++){
                pInode->dirBlockPtr[i] = -1;
            }
            pInode->indirectBlockPtr = -1;
            PutInode(targetInodeNum, pInode);

            //**---Set bytemap---**
            SetInodeBytemap(targetInodeNum);

            //**---FileSysInfo handle---**
            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
            _pFileSysInfo->numAllocInodes++;
            DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

            int findex = -1;
            int descInx = -1;
            for(int i=0; i<MAX_FILE_NUM; i++){
                if(_pFileTable->pFile[i].bUsed == 0){
                    _pFileTable->pFile[i].bUsed = 1;
                    _pFileTable->pFile[i].fileOffset = 0;
                    _pFileTable->pFile[i].inodeNum = targetInodeNum;
                    findex = i;
                    break;
                }
            }
            if(findex == -1) return -1;
            _pFileTable->numUsedFile++;
            for(int i=0;i<DESC_ENTRY_NUM;i++){
                if(_pFileDescTable->pEntry[i].bUsed == 0){
                    _pFileDescTable->pEntry[i].bUsed = 1;
                    _pFileDescTable->pEntry[i].fileTableIndex = findex;
                    descInx = i;
                    break;
                }
            }
            if(descInx == -1) return -1;
            _pFileDescTable->numUsedDescEntry++;
            free(pEntry);
            free(pInode);
            free(buf);
            
            return descInx;
        }
    }
    else if(flag == OPEN_FLAG_TRUNCATE){
       //1. Check Possible
        int check = 0;
        int nextInodeNum = 0;
        int prevInodeNum = 0;
        int targetdirBlkPtr = -1;
        int targetIndex = -1;
        int targetIndirBlkPtr = -1;
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

            if(check < i+1 && targetdirBlkPtr > 2)
            {
                targetdirBlkPtr = -1;
                for(int j=0;j<128;j++){
                    int find = 0;
                    targetIndirBlkPtr = j;
                    DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,j), pBuf);
                    for(int k=0; k< NUM_OF_DIRENT_PER_BLK; k++){
                        //if name match
                        if(strcmp(paths[i],pEntry[k].name)==0){
                        //find next Inode num
                            prevInodeNum = nextInodeNum;
                            targetIndex = k;
                            nextInodeNum = pEntry[k].inodeNum;
                            GetInode(nextInodeNum,pInode);
                            check++;
                            find = 1;
                            break;
                        }
                    }
                    if(find) break;
                }
            }
        }

        if(check != index) {
            free(pEntry);
            free(pInode);
            free(buf);
            return -1;
        }

        //Delete cur file
        pBuf = (char*) malloc(BLOCK_SIZE);
        
        for(int i=0;i<pInode->allocBlocks;i++){
            if(i<NUM_OF_DIRECT_BLOCK_PTR){
                DevReadBlock(pInode->dirBlockPtr[i], pBuf);
                memset(pBuf, 0, BLOCK_SIZE);
                DevWriteBlock(pInode->dirBlockPtr[i], pBuf);
                ResetBlockBytemap(pInode->dirBlockPtr[i]);
                pInode->dirBlockPtr[i] = -1;
                pInode->size -= BLOCK_SIZE;
                //Re set FileSysInfo
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                _pFileSysInfo->numFreeBlocks++;
                _pFileSysInfo->numAllocBlocks--;
                DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
            }
            else{
                DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4),pBuf);
                memset(pBuf, 0, BLOCK_SIZE);
                DevWriteBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4),pBuf);
                ResetBlockBytemap(GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4));
                RemoveIndirectBlockEntry(pInode->indirectBlockPtr, i-4);
                pInode->size -= BLOCK_SIZE;
                //Re set FileSysInfo
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                _pFileSysInfo->numFreeBlocks++;
                _pFileSysInfo->numAllocBlocks--;
                DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
            }
        }

        //Reset Cur Bytemap
        ResetInodeBytemap(nextInodeNum);

        //Reset Cur Inode
        pInode->allocBlocks = 0;
        PutInode(nextInodeNum, pInode);


        //Re set FileSysInfo
        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
        _pFileSysInfo->numAllocInodes--;
        DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

        free(pEntry);
        free(pInode);
        free(buf);
        
        int findex = -1;
        int descInx = -1;

        for(int i=0; i<MAX_FILE_NUM; i++){
            if(_pFileTable->pFile[i].bUsed == 0){
                _pFileTable->pFile[i].bUsed = 1;
                _pFileTable->pFile[i].fileOffset = 0;
                _pFileTable->pFile[i].inodeNum = nextInodeNum;
                findex = i;
                break;
            }
        }
        _pFileTable->numUsedFile++;
        if(findex == -1) return -1;
        for(int i=0;i<DESC_ENTRY_NUM;i++){
            if(_pFileDescTable->pEntry[i].bUsed == 0 && _pFileDescTable->pEntry[i].fileTableIndex == findex){
                _pFileDescTable->pEntry[i].bUsed = 1;
                descInx = i;
                break;
            }
        }
        _pFileDescTable->numUsedDescEntry++;
        return descInx;

        }
    return -1;
}


int WriteFile(int fileDesc, char* pBuffer, int length)
{
    
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    char* pBuf = (char*) malloc (BLOCK_SIZE);

    int ftIdx = _pFileDescTable->pEntry[fileDesc].fileTableIndex;
    int fInodeNum = _pFileTable->pFile[ftIdx].inodeNum;
    int offset = _pFileTable->pFile[ftIdx].fileOffset;
    int pBufOffset = 0;

    int EntryOffset = offset/ BLOCK_SIZE;
    int indexOffset = offset% BLOCK_SIZE;

    GetInode(fInodeNum, pInode);

    //estimate
    if(offset + length > pInode->allocBlocks * BLOCK_SIZE){ // need new block space
        int n_blkNum = 0;
        if((offset+length)%BLOCK_SIZE == 0)
            n_blkNum = (offset+length)/BLOCK_SIZE;
        else
            n_blkNum = (offset+length)/BLOCK_SIZE + 1;

        int makeNum = n_blkNum - pInode->allocBlocks;

        //alloc dirent block
        for(int i=0; i<NUM_OF_DIRECT_BLOCK_PTR;i++){
            if(pInode->dirBlockPtr[i] == -1){
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
                pInode->dirBlockPtr[i] = newBlkNum;
                PutInode(fInodeNum, pInode);

                //set bytemap
                SetBlockBytemap(newBlkNum);    
            
                //update FileSystemInfo
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                _pFileSysInfo->numFreeBlocks--;
                _pFileSysInfo->numAllocBlocks++;
                DevWriteBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

                free(n_pEntry);

                makeNum--;
                if(makeNum==0) break;
            }
        }
        
        if(makeNum>0){
            for(int i=0;i<128;i++){
                if(pInode->indirectBlockPtr == -1){ // make new Idx block
                    int idxBlkNum = GetFreeBlockNum();
                    pInode->indirectBlockPtr = idxBlkNum;
                    PutInode(fInodeNum,pInode);
                
                    int* pIdx = (int*)malloc(sizeof(int)*128);
                    DevReadBlock(idxBlkNum,(char*)pIdx);
                    for(int j=0;j<128;j++){
                        pIdx[j] = INVALID_ENTRY;
                    }
                    DevWriteBlock(idxBlkNum,(char*)pIdx);

                    //set bytemap
                    SetBlockBytemap(idxBlkNum);

                    //**---FileSysInfo handle---**
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    _pFileSysInfo->numFreeBlocks--;
                    DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    free(pIdx);
                }

                if(GetIndirectBlockEntry(pInode->indirectBlockPtr,i)==INVALID_ENTRY){ // allocate new indirect block
                    //allocate new block
                    DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                    char* n_pBuf = (char*) n_pEntry;
                    int nBlkNum = GetFreeBlockNum();

                    for(int m=0;m<NUM_OF_DIRENT_PER_BLK;m++){
                    memset(n_pEntry[m].name, 0, MAX_NAME_LEN);
                        n_pEntry[m].inodeNum = INVALID_ENTRY;
                    }
                    //init block
                    DevWriteBlock(nBlkNum, n_pBuf);
                    
                    //update Inode
                    pInode->allocBlocks++;
                    PutInode(fInodeNum, pInode);

                    //updateindirect block
                    PutIndirectBlockEntry(pInode->indirectBlockPtr, i, nBlkNum);
                    
                    //set bytemap
                    SetBlockBytemap(nBlkNum);    
                
                    //update FileSystemInfo
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    _pFileSysInfo->numFreeBlocks--;
                    _pFileSysInfo->numAllocBlocks++;
                    DevWriteBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

                    free(n_pEntry);
                    makeNum--;
                    if(makeNum == 0) break;
                }                
            }
        }
        
    }
    int s_blkNum;
    //copy
    for(int i= EntryOffset; i<pInode->allocBlocks;i++){
        if(i<NUM_OF_DIRECT_BLOCK_PTR)
            s_blkNum = pInode->dirBlockPtr[i];
        else
            s_blkNum = GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4);

        DevReadBlock(s_blkNum, pBuf);

        int copysize = 0;
        if(length >= BLOCK_SIZE - indexOffset)
            copysize = BLOCK_SIZE - indexOffset;
        else
            copysize = length;
        
        memcpy(pBuf+indexOffset, pBuffer + pBufOffset, copysize);
        pBufOffset += copysize;
        indexOffset = 0;
        length -= copysize;
        DevWriteBlock(s_blkNum, pBuf);
    }

    //set size
    pInode->size += pBufOffset;
    PutInode(fInodeNum,pInode);

    //set File table
    _pFileTable->pFile[ftIdx].fileOffset += pBufOffset;

    free(pInode);
    free(pBuf);

    return pBufOffset;
}


int ReadFile(int fileDesc, char* pBuffer, int length)
{
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    char* pBuf = (char*) malloc (BLOCK_SIZE);

    int ftIdx = _pFileDescTable->pEntry[fileDesc].fileTableIndex;
    int fInodeNum = _pFileTable->pFile[ftIdx].inodeNum;
    int offset = _pFileTable->pFile[ftIdx].fileOffset;

    int pBufOffset = 0;
    int EntryOffset = offset/ BLOCK_SIZE;
    int indexOffset = offset% BLOCK_SIZE;

    GetInode(fInodeNum, pInode);
    int s_blkNum;
    //copy
    for(int i= EntryOffset; i<pInode->allocBlocks;i++){
        if(i<NUM_OF_DIRECT_BLOCK_PTR)
            s_blkNum = pInode->dirBlockPtr[i];
        else
            s_blkNum = GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4);

        DevReadBlock(s_blkNum, pBuf);

        int copysize = 0;
        if(length >= BLOCK_SIZE - indexOffset)
            copysize = BLOCK_SIZE - indexOffset;
        else
            copysize = length;
        
        memcpy(pBuffer+pBufOffset, pBuf + indexOffset, copysize);
        pBufOffset += copysize;
        indexOffset = 0;
        length -= copysize;
    }

    PutInode(fInodeNum,pInode);

    //set File table
    _pFileTable->pFile[ftIdx].fileOffset += pBufOffset;

    free(pInode);
    free(pBuf);

    return pBufOffset;
}

int CloseFile(int fileDesc)
{
    if(_pFileDescTable->pEntry[fileDesc].bUsed == 0) return -1;
    int ftIdx = _pFileDescTable->pEntry[fileDesc].fileTableIndex;
    _pFileDescTable->pEntry[fileDesc].bUsed = 0;
    _pFileDescTable->numUsedDescEntry--;

    if(_pFileTable->numUsedFile == 0) return -1;
    else if(_pFileTable->pFile[ftIdx].bUsed == 0)return -1;
    _pFileTable->numUsedFile--;
    _pFileTable->pFile[ftIdx].bUsed = 0;

    return 0;
}

int RemoveFile(char* name)
{
    int index = 0;
    //extract path & dir name
    char* buf = (char*)malloc(MAX_NAME_LEN);
    char* paths[500] = {NULL, };
    strcpy(buf,name);
    buf = strtok(buf,"/");
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
    DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry));
    char* pBuf = (char*) pEntry;


    //1. Check Possible
    int check = 0;
    int nextInodeNum = 0;
    int prevInodeNum = 0;
    int targetdirBlkPtr = -1;
    int targetIndex = -1;
    int targetIndirBlkPtr = -1;
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

        if(check < i+1 && targetdirBlkPtr > 2)
        {
            targetdirBlkPtr = -1;
            for(int j=0;j<128;j++){
                int find = 0;
                targetIndirBlkPtr = j;
                DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,j), pBuf);
                for(int k=0; k< NUM_OF_DIRENT_PER_BLK; k++){
                    //if name match
                    if(strcmp(paths[i],pEntry[k].name)==0){
                    //find next Inode num
                        prevInodeNum = nextInodeNum;
                        targetIndex = k;
                        nextInodeNum = pEntry[k].inodeNum;
                        GetInode(nextInodeNum,pInode);
                        check++;
                        find = 1;
                        break;
                    }
                }
                if(find) break;
            }
        }
    }

    if(check != index) {
        free(pEntry);
        free(pInode);
        free(buf);
        return -1;
    }

    //Delete cur file
    pBuf = (char*) malloc(BLOCK_SIZE);
    
    for(int i=0;i<pInode->allocBlocks;i++){
        if(i<NUM_OF_DIRECT_BLOCK_PTR){
            DevReadBlock(pInode->dirBlockPtr[i], pBuf);
            memset(pBuf, 0, BLOCK_SIZE);
            DevWriteBlock(pInode->dirBlockPtr[i], pBuf);
            ResetBlockBytemap(pInode->dirBlockPtr[i]);
            pInode->dirBlockPtr[i] = -1;
            //Re set FileSysInfo
            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
            _pFileSysInfo->numFreeBlocks++;
            _pFileSysInfo->numAllocBlocks--;
            DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
        }
        else{
            DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4),pBuf);
            memset(pBuf, 0, BLOCK_SIZE);
            DevWriteBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4),pBuf);
            ResetBlockBytemap(GetIndirectBlockEntry(pInode->indirectBlockPtr, i-4));
            RemoveIndirectBlockEntry(pInode->indirectBlockPtr, i-4);
            //Re set FileSysInfo
            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
            _pFileSysInfo->numFreeBlocks++;
            _pFileSysInfo->numAllocBlocks--;
            DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
            DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
        }
    }

    //Reset Cur Bytemap
    ResetInodeBytemap(nextInodeNum);

    //Reset Cur Inode
    memset(pInode, 0, sizeof(Inode));
    PutInode(nextInodeNum, pInode);


    //Re set FileSysInfo
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
    _pFileSysInfo->numAllocInodes--;
    DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

    //reset desc table
    int findex = -1;
    int descInx = -1;

    for(int i=0; i<MAX_FILE_NUM; i++){
        if(_pFileTable->pFile[i].inodeNum == nextInodeNum && _pFileTable->pFile[i].bUsed == 1){
            _pFileTable->pFile[i].bUsed = 0;
            _pFileTable->pFile[i].fileOffset = 0;
            _pFileTable->pFile[i].inodeNum = -1;
            findex = i;
            _pFileTable->numUsedFile--;
            break;
        }
    }

    for(int i=0;i<DESC_ENTRY_NUM;i++){
        if(_pFileDescTable->pEntry[i].bUsed == 1 && _pFileDescTable->pEntry[i].fileTableIndex == findex){
            _pFileDescTable->pEntry[i].bUsed = 0;
            _pFileDescTable->numUsedDescEntry--;
            break;
        }
    }

    //Delete cur file in prev dir
    GetInode(prevInodeNum, pInode);
    if(targetdirBlkPtr != -1)
        RemoveDirEntry(pInode->dirBlockPtr[targetdirBlkPtr], targetIndex);
    else if(targetIndirBlkPtr != -1)
        RemoveDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr,targetIndirBlkPtr), targetIndex);

    //re construct prevdir entrys

    if(targetdirBlkPtr != -1) {
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
    }

    else if(targetIndirBlkPtr != -1){
        for(int i=targetIndirBlkPtr;i<128;i++){
        if(GetIndirectBlockEntry(pInode->indirectBlockPtr, i)==INVALID_ENTRY) break;
        
        int finish = 0;
        if(targetIndex == NUM_OF_DIRENT_PER_BLK -1){
            if(i!= 127 && GetIndirectBlockEntry(pInode->indirectBlockPtr, i+1) != INVALID_ENTRY){
                GetDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i+1),0,n_pEntry);
                PutDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i),targetIndex,n_pEntry);
                RemoveDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i+1),0);
                targetIndex = 0;
            }
        }

        for(int j=targetIndex+1;j<NUM_OF_DIRENT_PER_BLK;j++){
            GetDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i), j, n_pEntry);
            if(n_pEntry->inodeNum == INVALID_ENTRY){ //finish
                if(j == 1){ // reset prev Inode
                    
                    pInode->allocBlocks--;
                    RemoveIndirectBlockEntry(pInode->indirectBlockPtr, i);
                    pInode->size -= BLOCK_SIZE;
                    PutInode(prevInodeNum,pInode);

                    if(i == 0){ //reset idx blk
                        DevReadBlock(pInode->indirectBlockPtr,pBuf);
                        memset(pEntry,0,sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                        DevWriteBlock(pInode->indirectBlockPtr,pBuf);
                        //set bytemap
                        ResetBlockBytemap(pInode->indirectBlockPtr);

                        pInode->indirectBlockPtr = -1;
                        PutInode(prevInodeNum,pInode);

                        //**---FileSysInfo handle---**
                        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                        _pFileSysInfo->numFreeBlocks++;
                        DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    }

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
            PutDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i),j-1,n_pEntry);
            RemoveDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i),j);
        }
        if(finish) break;
        targetIndex = NUM_OF_DIRENT_PER_BLK-1;
        }
    }
    free(n_pEntry);
    free(pEntry);
    free(pInode);
    free(buf);
    return 0;
}


int MakeDirectory(char* name)
{
    int index = 0;
    //extract path & dir name
    char* buf = (char*)malloc(MAX_NAME_LEN);
    char* paths[500] = {NULL, };
    strcpy(buf,name);
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
    int targetIndirBlkPtr = -1;
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
        if(check < i+1 && targetdirBlkPtr > 2) {
            targetdirBlkPtr = -1;
            if(pInode->indirectBlockPtr==-1){
                //allocate index block
                int idxBlkNum = GetFreeBlockNum();
                pInode->indirectBlockPtr = idxBlkNum;
                PutInode(nextInodeNum,pInode);
            
                int* pIdx = (int*)malloc(sizeof(int)*128);
                DevReadBlock(idxBlkNum,(char*)pIdx);
                for(int j=0;j<128;j++){
                    pIdx[j] = INVALID_ENTRY;
                }
                DevWriteBlock(idxBlkNum,(char*)pIdx);

                //set bytemap
                SetBlockBytemap(idxBlkNum);

                //**---FileSysInfo handle---**
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                _pFileSysInfo->numFreeBlocks--;
                DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                free(pIdx);
            }   
            //serach in idxblcks
 
            for(int j=0;j<128;j++){
                int find = 0;

                if(i == index-1 && GetIndirectBlockEntry(pInode->indirectBlockPtr, j) == INVALID_ENTRY){ //allocate new block
                    //allocate new block
                    DirEntry* n_pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                    char* n_pBuf = (char*) n_pEntry;
                    int nBlkNum = GetFreeBlockNum();

                    for(int m=0;m<NUM_OF_DIRENT_PER_BLK;m++){
                    memset(n_pEntry[m].name, 0, MAX_NAME_LEN);
                        n_pEntry[m].inodeNum = INVALID_ENTRY;
                    }
                    //init block
                    DevWriteBlock(nBlkNum, n_pBuf);
                    
                    //update Inode
                    pInode->allocBlocks++;
                    pInode->size += BLOCK_SIZE;
                    PutInode(nextInodeNum, pInode);

                    //updateindirect block
                    PutIndirectBlockEntry(pInode->indirectBlockPtr, j, nBlkNum);
                    

                    //set bytemap
                    SetBlockBytemap(nBlkNum);    
                
                    //update FileSystemInfo
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    _pFileSysInfo->numFreeBlocks--;
                    _pFileSysInfo->numAllocBlocks++;
                    DevWriteBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);

                    free(n_pEntry);
                }
                
                targetIndirBlkPtr = j;
                DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,j), pBuf);
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
    if(targetdirBlkPtr !=-1){
        DevReadBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);
        strncpy(pEntry[targetDirEntryNum].name,paths[index-1],strlen(paths[index-1])+1);
        pEntry[targetDirEntryNum].inodeNum = targetInodeNum;
        DevWriteBlock(pInode->dirBlockPtr[targetdirBlkPtr],pBuf);
    }
    else if(targetIndirBlkPtr != -1 && targetdirBlkPtr == -1){
        DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,targetIndirBlkPtr),pBuf);
        strncpy(pEntry[targetDirEntryNum].name,paths[index-1],strlen(paths[index-1])+1);
        pEntry[targetDirEntryNum].inodeNum = targetInodeNum;
        DevWriteBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,targetIndirBlkPtr),pBuf);
    }
    

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
    pInode->indirectBlockPtr = -1;
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
    strcpy(buf,name);
    buf = strtok(buf,"/");
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
    int targetIndirBlkPtr = -1;
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

        if(check < i+1 && targetdirBlkPtr > 2)
        {
            targetdirBlkPtr = -1;
            for(int j=0;j<128;j++){
                int find = 0;
                targetIndirBlkPtr = j;
                DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,j), pBuf);
                for(int k=0; k< NUM_OF_DIRENT_PER_BLK; k++){
                    //if name match
                    if(strcmp(paths[i],pEntry[k].name)==0){
                    //find next Inode num
                        prevInodeNum = nextInodeNum;
                        targetIndex = k;
                        nextInodeNum = pEntry[k].inodeNum;
                        GetInode(nextInodeNum,pInode);
                        check++;
                        find = 1;
                        break;
                    }
                }
                if(find) break;
            }
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
    if(targetdirBlkPtr != -1)
        RemoveDirEntry(pInode->dirBlockPtr[targetdirBlkPtr], targetIndex);
    else if(targetIndirBlkPtr != -1)
        RemoveDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr,targetIndirBlkPtr), targetIndex);

    //re construct prevdir entrys

    if(targetdirBlkPtr != -1) {
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
    }

    else if(targetIndirBlkPtr != -1){
        for(int i=targetIndirBlkPtr;i<128;i++){
        if(GetIndirectBlockEntry(pInode->indirectBlockPtr, i)==INVALID_ENTRY) break;
        
        int finish = 0;
        if(targetIndex == NUM_OF_DIRENT_PER_BLK -1){
            if(i!= 127 && GetIndirectBlockEntry(pInode->indirectBlockPtr, i+1) != INVALID_ENTRY){
                GetDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i+1),0,n_pEntry);
                PutDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i),targetIndex,n_pEntry);
                RemoveDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i+1),0);
                targetIndex = 0;
            }
        }

        for(int j=targetIndex+1;j<NUM_OF_DIRENT_PER_BLK;j++){
            GetDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i), j, n_pEntry);
            if(n_pEntry->inodeNum == INVALID_ENTRY){ //finish
                if(j == 1){ // reset prev Inode
                    
                    pInode->allocBlocks--;
                    RemoveIndirectBlockEntry(pInode->indirectBlockPtr, i);
                    pInode->size -= BLOCK_SIZE;
                    PutInode(prevInodeNum,pInode);

                    if(i == 0){ //reset idx blk
                        DevReadBlock(pInode->indirectBlockPtr,pBuf);
                        memset(pEntry,0,sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
                        DevWriteBlock(pInode->indirectBlockPtr,pBuf);
                        //set bytemap
                        ResetBlockBytemap(pInode->indirectBlockPtr);

                        pInode->indirectBlockPtr = -1;
                        PutInode(prevInodeNum,pInode);

                        //**---FileSysInfo handle---**
                        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                        _pFileSysInfo->numFreeBlocks++;
                        DevWriteBlock(FILESYS_INFO_BLOCK,(char*)_pFileSysInfo);
                        DevReadBlock(FILESYS_INFO_BLOCK, (char*)_pFileSysInfo);
                    }

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
            PutDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i),j-1,n_pEntry);
            RemoveDirEntry(GetIndirectBlockEntry(pInode->indirectBlockPtr, i),j);
        }
        if(finish) break;
        targetIndex = NUM_OF_DIRENT_PER_BLK-1;
        }
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

    _pFileDescTable = (FileDescTable*)malloc(sizeof(FileDescTable));
    memset(_pFileDescTable,0,sizeof(FileDescTable));

    _pFileTable = (FileTable*)malloc(sizeof(FileTable));
    memset(_pFileTable, 0 , sizeof(FileTable));

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
    pInode->indirectBlockPtr = -1;
    
    //Put target num Inode
    PutInode(targetInodeNum, pInode);

    //free(pFileSysInfo);
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

    _pFileDescTable = (FileDescTable*)malloc(sizeof(FileDescTable));
    memset(_pFileDescTable,0,sizeof(FileDescTable));

    _pFileTable = (FileTable*)malloc(sizeof(FileTable));
    memset(_pFileTable, 0 , sizeof(FileTable));

}

void CloseFileSystem(void)
{
    free(_pFileSysInfo);
    free(_pFileDescTable);
    free(_pFileTable);
    DevCloseDisk();
}

Directory* OpenDirectory(char* name)
{
    int index = 0;
    //extract path & dir name
    char* buf = (char*)malloc(MAX_NAME_LEN);
    char* paths[500] = {NULL, };
    strcpy(buf, name);
    buf = strtok(buf,"/");
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
    int targetIndirBlkPtr = -1;

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
        if(check < i+1 && targetdirBlkPtr > 2)
        {
            for(int j=0;j<128;j++){
                int find = 0;
                targetIndirBlkPtr = j;
                DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,j), pBuf);
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
                }
                if(find) break;
            }
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

    free(pEntry);
    free(pInode);
    free(buf);
    return dir;
}


FileInfo* ReadDirectory(Directory* pDir)
{
    Directory* _pDir = (Directory*)malloc(sizeof(Directory));
    memcpy(_pDir,pDir,sizeof(Directory));
    //2. return FileInfo
    int _inodeNum = _pDir->inodeNum;
    Inode* pInode = (Inode*)malloc(sizeof(Inode));
    DirEntry* pEntry = (DirEntry*)malloc(sizeof(DirEntry)*NUM_OF_DIRENT_PER_BLK);
    FileInfo* pFileInfo = (FileInfo*)malloc(sizeof(FileInfo));
    memset(pFileInfo,0,sizeof(FileInfo));
    GetInode(_inodeNum,pInode);

    //search
    int curDirPtr = offset/ NUM_OF_DIRENT_PER_BLK;
    int curEntry = offset% NUM_OF_DIRENT_PER_BLK;
    int useIndir = 0;
    if(curDirPtr >= NUM_OF_DIRECT_BLOCK_PTR){
        useIndir = 1;
        curDirPtr = (offset -32)/ NUM_OF_DIRENT_PER_BLK;
        curEntry = (offset-32)% NUM_OF_DIRENT_PER_BLK;
        if(curDirPtr >= 128){
            offset = 0;
            return NULL;
        }
    }

    if(pInode->dirBlockPtr[curDirPtr] == -1){
        offset = 0;
        return NULL;
    }
    if(useIndir == 0)
        DevReadBlock(pInode->dirBlockPtr[curDirPtr], (char*)pEntry);
    else if(useIndir == 1)
        DevReadBlock(GetIndirectBlockEntry(pInode->indirectBlockPtr,curDirPtr), (char*)pEntry);

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
    free(_pDir);
    free(pInode);
    free(pEntry);

    return pFileInfo;
}

int CloseDirectory(Directory* pDir)
{
    free(pDir);
    return 0;
}
