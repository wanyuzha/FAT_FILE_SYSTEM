#include "fat_file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int show_dir() {
	FILE* fp;
	fp = fopen("vdisk", "rb");
	extern int sector_per_FAT_int;
	extern int sector_per_cluster_int;
	extern int dir_index;
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	int catalog_num = sector_per_cluster_int * SECTOR / sizeof(catalog);
	for (int i = 0;i < catalog_num;i++) {
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		if (c.is_valid == 1) {
			if (i == 0) {
				printf("打开的文件夹\n%12s\t\t\t%s\t%s\t\n", c.filename, c.user_name, c.attribute);
			}
			else if (strcmp(c.filename_extension, "dir") == 0) {
				printf("%12s\t\t\t%s\t%s\t\n", c.filename, c.user_name, c.attribute);
			}
			else
			{
				printf("%8s.%4s\t\t%4d\t%s\t%s\t\n", c.filename, c.filename_extension, c.file_size, c.user_name, c.attribute);
			}

		}
	}
	fclose(fp);
	return 0;
}

/* 专门用于移动文件指针查找簇 */
int print_cluster(FILE* fp, int sector_per_FAT_int, int sector_per_cluster_int, int cluster_index_int) {
	char* buffer = (char*)malloc(SECTOR*sector_per_cluster_int);
	long offset = SECTOR * 64 + sector_per_FAT_int * 2 * SECTOR + sector_per_cluster_int * SECTOR*cluster_index_int;
	fseek(fp, offset, SEEK_SET);
	fread(buffer, SECTOR*sector_per_cluster_int, 1, fp);
	printf("%s", buffer);
	return 0;
}

int read_file(char* filename) {
	extern int sector_per_FAT_int;
	extern int sector_per_cluster_int;
	extern int dir_index;

	int catalog_index;
	find_file(filename, catalog_index);
	if (catalog_index == -1) {
		return -1;
	}
	/* 按簇读取文件内容 */
	FILE* fp;
	fp = fopen("vdisk", "rb");
	catalog c;
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(c)), SEEK_SET);
	fread(&c, sizeof(c), 1, fp);
	/* 不是文件结束簇，一直向下，直到找到文件结束簇 */
	char cluster_index[2];
	int cluster_index_int = (c.start_cluster[0] << 8) + c.start_cluster[1];
	fseek(fp, SECTOR * 64 + cluster_index_int * 2, SEEK_SET);
	fread(cluster_index, sizeof(cluster_index), 1, fp);

	/* 没有将cluster_index转无符号数，暂时用-1代替 */
	while (cluster_index[0] != -1 || cluster_index[1] != -1) {
		print_cluster(fp, sector_per_FAT_int, sector_per_cluster_int, cluster_index_int);
		cluster_index_int = (cluster_index[0] << 8) + cluster_index[1];
		fseek(fp, SECTOR * 64 + cluster_index_int * 2, SEEK_SET);
		fread(cluster_index, sizeof(cluster_index), 1, fp);
	}
	/* 显示该文件的最后一簇 */
	print_cluster(fp, sector_per_FAT_int, sector_per_cluster_int, cluster_index_int);
	fclose(fp);
	return 0;
}

int file_system() {
	FILE* fp;
	fp = fopen("vdisk", "r");
	if (fp == NULL) {
		printf("请先创建磁盘文件\n");
		return 0;
	}
	fclose(fp);
	file_param_initial();
	int is_valified = 0;
	while (true)
	{
		if (!is_valified) {
			printf("***************FAT文件系统*******************\n");
			printf("***************用户登陆(login)***************\n");
			printf("***************返回上一级目录(cd ..)**********\n");
			getchar();
			char command[COMMAND_MAX_LENGTH];
			gets_s(command);
			char *args[ARGS_MAX_NUM] = { 0 };
			int num = 0;
			split(command, " ",args, &num);
			if (num > 0) {
				if (strncmp(args[0], "login", 5) == 0) {
					is_valified = valify_user(args);
				}
				else if (strncmp(args[0], "cd", 2) == 0) {
					if (strncmp(args[1], "..", 2) == 0) {
						return 0;
					}
				}
			}
		}
		if (is_valified) {
			extern char active_user[];
			printf("当前的用户是%s\n", active_user);
			printf("----------------FAT文件系统------------------\n");
			/*
			printf("***************1.用户切换********************\n");
			printf("***************2.创建用户********************\n");
			printf("***************3.删除用户********************\n");
			printf("***************4.查看文件或目录**************\n");
			printf("***************5.创建文件********************\n");
			printf("***************6.删除文件********************\n");
			printf("***************7.移动文件********************\n");
			printf("***************8.文件改名********************\n");
			printf("***************9.返回上一级******************\n");
			*/
			char command[COMMAND_MAX_LENGTH];
			gets_s(command);
			char *args[ARGS_MAX_NUM] = { 0 };
			int num = 0;
			split(command, " ",args, &num);
			if (num > 0) {
				if (strncmp(args[0], "runas", 5) == 0) {
					is_valified = valify_user(args);
				}
				else if (strncmp(args[0], "dir", 3) == 0) {
					show_dir();
				}
				else if (strncmp(args[0], "net", 3) == 0 && strncmp(args[1], "user", 3) == 0 && strncmp(args[4], "/add", 3) == 0) {
					create_user(args);
				}
				else if (strncmp(args[0], "type", 4) == 0 && strncmp(args[1], "nul>", 4) == 0) {
					if (num >= 3) {
						if (create_file(args) == -1) {
							printf("只有root用户才可以创建新的用户,请咨询root用户以创建");
						}
					}
				}
				else if (strncmp(args[0], "type", 4) == 0) {
					if (read_file(args[1]) == -1) {
						printf("您输入的文件名有误，请重新输入");
					}
					else
					{
						printf("\n");
					}
				}
				else if (strncmp(args[0], "write", 5) == 0) {
					if (num == 3) {
						if (write_file(args) == -1) {
							printf("您输入的路径名有误，请重新输入");
						}
					}
				}
				else if (strncmp(args[0], "cd", 2) == 0) {
					if (strncmp(args[1], "..", 2) == 0) {
						extern int dir_index;
						if (dir_index == 2) {
							return 0;
						}
						return_dir();
						show_dir();
					}
					else
					{
						if (read_dir(args[1]) == -1) {
							printf("没有找到此文件夹");
						}
						else if (read_dir(args[1]) == -2) {
							printf("您没有权限操作此文件夹");
						}
						show_dir();
					}
				}
				else if (strncmp(args[0], "md", 2) == 0) {
					create_dir(args);
				}
				else if (strncmp(args[0], "rename", 6) == 0) {
					if (rename(args[1], args[2]) == -1) {
						printf("没有找到文件或文件夹");
					}
					else if (rename(args[1], args[2]) == -2) {
						printf("您没有权限操作此文件或文件夹");
					}
				}
				else if (strncmp(args[0], "del", 3) == 0) {
					if (delete_file(args[1]) == -1) {
						printf("没有找到文件");
					}
					else if (delete_file(args[1]) == -2) {
						printf("没有权限操作此文件");
					}
				}
				else if (strncmp(args[0], "rd", 2) == 0) {
					if (delete_dir(args[1]) == -1) {
						printf("没有找到文件夹");
					}
					else if (delete_dir(args[1]) == -2) {
						printf("没有权限操作此文件夹");
					}
				}
				else if (strcmp(args[0], "move") == 0) {
					if (num < 3) {
						printf("参数不足");
					}
					if (move_file(args) == -1) {
						printf("找不到文件或路径错误");
					}
				}
			}
		}
		else
		{
			printf("登陆失败");
			return 0;
		}
		printf("--------------------------------------------\n");
	}
	return 0;
}

/* 同时展示FAT表内部结构 */
int check_disk() {
	FILE* fp;
	fp = fopen("vdisk", "rb");

	unsigned char sector_per_cluster[1];
	fseek(fp, (long)(SECTOR * 63 + 0x0d), SEEK_SET);
	fread(sector_per_cluster, sizeof(sector_per_cluster), 1, fp);
	int sector_per_cluster_int = hex_char_dex(sector_per_cluster, sizeof(sector_per_cluster));

	unsigned char _size[2];
	fread(_size, sizeof(_size), 1, fp);
	int size = (int)(_size[0] << 8) + (int)_size[1];
	int sector_per_FAT_int = (size * BUFFER_SIZE) / (SECTOR*(int)sector_per_cluster[0] * 1 << 8);

	int cluster_taken = 0;

	fseek(fp, (long)(SECTOR * 64 + 4), SEEK_SET);
	char cluster_status[2];
	for (int i = 0;i < sector_per_FAT_int*SECTOR / 2 - 2;i++) {
		fread(cluster_status, sizeof(cluster_status), 1, fp);
		if (cluster_status[0] != 0 || cluster_status[1] != 0) {
			cluster_taken++;
			int cluster_index = (cluster_status[0] << 8) + cluster_status[1];
			printf("第%d簇被占用,内容为%d\n", i + 2, cluster_index);
		}
	}

	int sector_left = size * BUFFER_SIZE / SECTOR - 64 - cluster_taken * 2 * sector_per_cluster_int;

	printf("目前共有%d簇被占用\n", cluster_taken);
	printf("剩余的扇区数为%d\n", sector_left);
	printf("剩余空间为%d(单位M)\n", sector_left*SECTOR / BUFFER_SIZE);
	fclose(fp);
	return 0;
}