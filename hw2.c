#include <stdio.h>
#include <stdlib.h>
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
