#include"allocate.h"
#include<stdio.h>
char buffer[BUFFER_SIZE];

int allocate_disk() {
	int size;
	FILE* fp;
	printf("请输入要创建的虚拟磁盘大小:(单位为M)");
	scanf("%d", &size);
	fp = fopen("vdisk", "w");
	for (int i = 0;i < size;i++) {
		fwrite(buffer, BUFFER_SIZE, 1, fp);
	}
	fclose(fp);
	return 0;
}