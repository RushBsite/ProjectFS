#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include "fs.h"

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

void PutDirEntry(int blkno, int index, DirEntry* pEntry)
{

}

void GetDirEntry(int blkno, int index, DirEntry* pEntry)
{

}
