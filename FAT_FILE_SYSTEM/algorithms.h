#pragma once
#define BUFFER_SIZE (1<<20)
#define SECTOR (1<<9)
#include <time.h>
#include <stdio.h>
#define COMMAND_MAX_LENGTH 2048
#define ARGS_MAX_NUM 8

struct Time
{
	char time[3];
	char date[3];
	int precise_time;
};

Time query_time();

int hex_char_dex(unsigned char* hex, int length);

int split(char *src, const char *separator, char **dest, int *num);

int not_allocate_FAT_index(FILE* fp, char* cluster_index, int sector_per_FAT_int);

int check_disk();

int string_copy(char* dest, char* src, int length);

int time_to_char(char* dest);

//int print_cluster(FILE* fp, int sector_per_FAT_int, int sector_per_cluster_int, int cluster_index_int);