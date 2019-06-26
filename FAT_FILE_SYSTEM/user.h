#pragma once
#include "algorithms.h"


/* 一个userfile占用64个字节 */
struct user_file
{
	char num;
	char address[32] = {0};
	char unuse[31];
};


/* 一个user占用64字节 */

struct user
{
	char user_name[10];
	char password[16];
	char is_valid;
	char is_system;
	char establish_date[3];
	char establish_time[3];
	char unuse[30];
};

/* 如果想要方便地解决文件的共享，可能要使用二级目录来进行管理 */

int valify_user(char** args);
int create_user(char** args);
int delete_user();