#include "fs.h"
#include "disk.h"
#include <stdio.h>

int main(){
	printf("Hello OS World\n");
	FileSysInit();
	SetInodeBytemap(0);
	return 0;
}
