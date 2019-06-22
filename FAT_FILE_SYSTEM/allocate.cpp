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
	/* 构建MBR分区 */
	fp = fopen("vdisk", "r+");
	fseek(fp, (long)SECTOR - 4, SEEK_CUR);
	char end[] = { '5', '5', 'a', 'a' };
	fwrite(end, sizeof(end), 1, fp);
	

	/*	*构建DBR分区 */
	fclose(fp);

	/* 究竟是否需要构建MBR分区和DBR分区的意义尚不明确，按照要求率先预留前64个扇区 */

	return 0;
}