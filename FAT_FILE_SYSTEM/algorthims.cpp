#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "algorithms.h"


int hex_char_dex(unsigned char* hex, int length) {
	int dex = 0;
	for (int i = 0;i < length;i++) {
		dex += (int)hex[i] * (int)pow(1 << 8, length - 1 - i);
	}
	return dex;
}

Time query_time() {
	Time return_time;
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep); //此函数获得的tm结构体的时间，是已经进行过时区转化为本地时间
	return_time.date[0] = p->tm_year;
	return_time.date[1] = p->tm_mon;
	return_time.date[2] = p->tm_mday;
	return_time.precise_time = 3600 * p->tm_hour + 60 * p->tm_min + p->tm_sec;
	return_time.time[0] = p->tm_hour;
	return_time.time[1] = p->tm_min;
	return_time.time[2] = p->tm_sec;
	return return_time;
	//printf("%d%02d%02d%02d%02d%02d\n", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

int split(char *src, const char *separator, char **dest, int *num) {
	/*
	src 源字符串的首地址(buf的地址)
	separator 指定的分割字符
	dest 接收子字符串的数组
	num 分割后子字符串的个数
	*/
	char *pNext;
	int count = 0;
	if (src == NULL || strlen(src) == 0) 
		/* 如果传入的地址为空或长度为0，直接终止  */
		return -1;
	if (separator == NULL || strlen(separator) == 0) 
		/* 如未指定分割的字符串，直接终止 */
		return -1;
	pNext = (char *)strtok(src, separator);
	/* 必须使用(char *)进行强制类型转换(虽然不写有的编译器中不会出现指针错误) */
	while (pNext != NULL) {
		*dest++ = pNext;
		++count;
		pNext = (char *)strtok(NULL, separator);  
		/* 必须使用(char *)进行强制类型转换 */
	}
	*num = count;
	return 0;
}

int not_allocate_FAT_index(FILE* fp, char* cluster_index, int sector_per_FAT_int) {
	char cluster_status[2];
	fseek(fp, (long)(SECTOR * 64 + 4), SEEK_SET);
	for (int i = 2;i < sector_per_FAT_int*SECTOR / 2 ;i++) {
		fread(cluster_status, sizeof(cluster_status), 1, fp);
		if (cluster_status[0] == 0 && cluster_status[1] == 0) {
			cluster_index[0] = i >> 8;
			cluster_index[1] = i;
			cluster_status[0] = 0xFF;
			cluster_status[1] = 0xFF;
			/* 一旦找到一个尚未被占用的簇，就把FFFF写回这个簇编号 */
			fseek(fp, -(long)sizeof(cluster_status), SEEK_CUR);
			fwrite(cluster_status, sizeof(cluster_status), 1, fp);
			/* 在FAT2表中填入同样的项目 */
			fseek(fp, (long)SECTOR*sector_per_FAT_int, SEEK_CUR);
			fwrite(cluster_status, sizeof(cluster_status), 1, fp);
			break;
		}
	}
	return 0;
}


/* 该方法将两个等长的字符数组进行拷贝，需预先知道长度 */
int string_copy(char* dest, char* src, int length) {
	for (int i = 0;i < length;i++) {
		dest[i] = src[i];
	}
	return 0;
}

/* 返回一个字符数组，数组中是秒数转换的字符串 */
int time_to_char(char* dest) {
	Time t = query_time();
	int time = t.precise_time;
	int i = 0;
	while (time!=0)
	{
		dest[i] = time % 10 + '0';
		time = time / 10;
		i++;
	}
	dest[i] = '\0';
	return 0;
}

