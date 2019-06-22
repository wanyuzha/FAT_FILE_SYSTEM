#include"initial.h"
#include<stdio.h>
#include<math.h>
#include<string.h>
char buffer[BUFFER_SIZE];

int allocate_disk();

int hex_char_dex(unsigned char* hex, int length) {
	int dex = 0;
	for (int i = 0;i < length;i++) {
		dex += (int)hex[i] * (int)pow(1<<8, length - 1 - i);
	}
	return dex;
}

int f_sector_per_cluster(int size) {
	if (size <= 0 || size > 4096) {
		return -1;
	}
	else if(size<33)
	{
		return 4;
	}
	else if(size<65)
	{
		return 4;
	}
	else if (size < 129)
	{
		return 4;
	}
	else if(size<257)
	{
		return 8;
	}
	else if(size<513)
	{
		return 16;
	}
	else if (size < 1025)
	{
		return 32;
	}
	else if(size<2049)
	{
		return 64;
	}
	else
	{
		return 128;
	}
}

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
	fseek(fp, (long)SECTOR - 2, SEEK_SET);
	unsigned char end[] = { 0x55, 0xaa };
	fwrite(end, sizeof(end), 1, fp);
	

	/*	* 构建DBR分区 
		* 构建BPB分区
		* 计算FAT表的长度，也就是计算FAT所占用的扇区数，然后将其写入BPB分区
	*/
	unsigned char bytes_per_sector[] = {0x02, 0x00};
	fseek(fp, (long)(SECTOR*62+0x0b), SEEK_CUR);
	fwrite(bytes_per_sector, sizeof(bytes_per_sector), 1, fp);
	unsigned char sector_per_cluster[] = { (char)f_sector_per_cluster(size) };
	fwrite(sector_per_cluster, sizeof(sector_per_cluster), 1, fp);
	int sector_per_FAT_int = (size*BUFFER_SIZE) / (SECTOR*(int)sector_per_cluster[0]*1<<8);
	unsigned char sector_per_FAT[2];
	sector_per_FAT[0] = (char)sector_per_FAT_int>>8;
	sector_per_FAT[1] = (char)sector_per_FAT_int;
	fseek(fp, (long)(0x16 - 0x0d), SEEK_CUR);
	fwrite(sector_per_FAT, sizeof(sector_per_FAT), 1, fp);
	/* 究竟是否需要构建MBR分区和DBR分区的意义尚不明确，按照要求率先预留前64个扇区 */

	/* * 构建FAT表 
	*/
	char not_allocate_cluster[] = { 0, 0 };
	fseek(fp, (long)(SECTOR * 64), SEEK_SET);
	for (int i = 0;i < sector_per_FAT_int*SECTOR;i++) {
		fwrite(not_allocate_cluster, sizeof(not_allocate_cluster), 2, fp);
	}
	fclose(fp);
	return 0;
}

int read_disk() {
	FILE* fp;
	fp = fopen("vdisk", "r");
	if (fp == NULL) {
		printf("请先创建磁盘文件\n");
		return 0;
	}
	unsigned char bytes_per_sector[2];
	fseek(fp, (long)(SECTOR * 63 + 0x0b), SEEK_SET);
	fread(bytes_per_sector, sizeof(bytes_per_sector), 1, fp);
	printf("扇区的大小是\t%d\n", hex_char_dex(bytes_per_sector, sizeof(bytes_per_sector)));
	unsigned char sector_per_cluster[1];
	fread(sector_per_cluster, sizeof(sector_per_cluster), 1, fp);
	printf("每一簇的大小是\t%d扇区\n", hex_char_dex(sector_per_cluster, sizeof(sector_per_cluster)));
	fclose(fp);
	return 0;
}

int initial() {
	int input;
	while (true)
	{
		int input;
		printf("***************初始化系统********************\n");
		printf("***************1.创建磁盘文件****************\n");
		printf("***************2.查看磁盘文件信息************\n");
		printf("***************0.返回上一级目录**************\n");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			allocate_disk();
			break;
		case 2:
			read_disk();
			break;
		case 3:
			return 0;
		default:
			break;
		}
	}
	return 0;
}