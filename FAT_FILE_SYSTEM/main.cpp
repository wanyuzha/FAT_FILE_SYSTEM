#include<stdio.h>
#include "initial.h"
#include<iostream>
using namespace std;

/*	* main文件用于测试各个模块的功能
	* 在控制台窗口选择想要的功能
	* 考虑到磁盘大小，构建FAT16文件系统
	*/

int main() {
	while (true)
	{
		int input;
		printf("***************FAT文件系统*******************\n");
		printf("***************1.初始化系统******************\n");
		printf("***************2.进入FAT文件系统*************\n");
		printf("***************3.关于***********************\n");
		printf("***************0.Exit***********************\n");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			initial();
			break;
		case 2:
			break;
		case 3:
			break;
		case 0:
			return 0;
		default:
			break;
		}
	}
	
	return 0;
}