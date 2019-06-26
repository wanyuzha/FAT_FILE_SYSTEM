#include <stdio.h>
#include "initial.h"
#include "version.h"
#include "fat_file_system.h"
#include "algorithms.h"
#include "file.h"
#include <iostream>
using namespace std;

/*	* main文件用于测试各个模块的功能
	* 在控制台窗口选择想要的功能
	* 考虑到磁盘大小，构建FAT16文件系统
	*/

/*	一定要注意的点：
	普通的字符串复制可以用strcpy()，但是如果字符串中含0，会自动停止，请使用string_copy()
	0xFF为-1*/
int main() {
	FILE* fp = fopen("vdisk", "rb");
	if (fp == NULL) {
		printf("请先初试化磁盘");
		initial();
	}
	while (true)
	{
		int input;
		printf("***************FAT文件系统*******************\n");
		printf("***************1.初始化系统******************\n");
		printf("***************2.进入FAT文件系统*************\n");
		printf("***************3.查看磁盘使用情况************\n");
		printf("***************4.关于***********************\n");
		printf("***************0.Exit***********************\n");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			initial();
			break;
		case 2:
			file_system();
			break;
		case 3:
			check_disk();
			break;
		case 4:
			print_version_details();
			break;
		case 0:
			return 0;
		default:
			break;
		}
	}
	
	return 0;
}