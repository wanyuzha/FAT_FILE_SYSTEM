#pragma once
#include "algorithms.h"

/*
char filename[8];				文件名
char filename_extension[3];		文件拓展名
char attribute[3];				属性
char establish_time_precise;	精确创建时间
char establish_time[2];			创建时间 (小时、分钟、秒)
char establish_date[2];			创建日期 (年、月、日)
char latest_access_date[2];		最近修改日期
char unuse2[2];					补齐
char modify_time[2];			修改时间
char modify_date[2];			修改日期
char start_cluster[2];			开始簇号
char file_size[4];				文件大小
char user_name[10];				所属用户
char former_cluster[2];			上一个簇号(用于返回上一级目录)
char unuse3[8];				补齐
*/

/*  struct中含int自动补齐4字节倍数 */
struct catalog
{
	char filename[8];
	char filename_extension[4];
	char attribute[3];
	char establish_time[3];
	char establish_date[3];
	char latest_access_date[3];
	char is_valid;
	char modify_time[3];
	char modify_date[3];
	char start_cluster[2];
	char user_name[10];
	char former_cluster[2];
	char unuse2[11];
	/* 上方凑够60字节 为4的倍数 */
	int file_size;
	int establish_time_precise;
};


int file_param_initial();

int create_dir(char** filename);
int create_file(char** filename);

//int show_dir();
int show_file();

int modify_file();

int delete_dir(char* filename);
int delete_file(char* filename);

int move_dir();

int move_file(char** args);

int read_dir(char* filename);

int return_dir();

int write_file(char** args);

int find_file(char* filename, int& catalog_index);

int test_file(char** filename);

int rename(char* filename, char* newname);

