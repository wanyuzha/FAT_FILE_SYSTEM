#include "user.h"
#include <string.h>

char active_user[10];
int user_num;
int sector_per_FAT_int;

int valify_user(char** args) {
	/* 
		用户查询算法设计 
		首先找到一簇的具体大小
		然后按照user结构体逐个访问数据区首簇，观察该有效位是否有效，如果有效就显示name
		*/
	FILE* fp;
	fp = fopen("vdisk", "r");

	unsigned char sector_per_cluster[1];
	fseek(fp, (long)(SECTOR * 63 + 0x0d), SEEK_SET);
	fread(sector_per_cluster, sizeof(sector_per_cluster), 1, fp);
	int sector_per_cluster_int = hex_char_dex(sector_per_cluster, sizeof(sector_per_cluster));
	sector_per_FAT_int = (10*BUFFER_SIZE) / (SECTOR*(int)sector_per_cluster[0] * 1 << 8);

	unsigned char _size[2];
	fread(_size, sizeof(_size), 1, fp);
	int size = (int)(_size[0] << 8) + (int)_size[1];

	user user_in;
	user_num = sector_per_cluster_int * SECTOR / sizeof(user_in);

	fseek(fp, (long)((64 + sector_per_FAT_int * 2)*SECTOR), SEEK_SET);
	/* 
	for (int i = 0;i < user_num;i++) {
		fread(&user_in, sizeof(user_in), 1, fp);
		if (user_in.is_valid == 1) {
			printf("\t%s\n", user_in.user_name);
		}
	}*/	

	/*
	printf("请输入用户名:");
	char user_name[sizeof(user_in.user_name)];
	scanf("%s", &user_name);*/
	fseek(fp, (long)((64 + sector_per_FAT_int * 2)*SECTOR), SEEK_SET);
	for (int i = 0;i < user_num;i++) {
		fread(&user_in, sizeof(user_in), 1, fp);
		if (strncmp(args[1], user_in.user_name, strlen(args[1]))==0) {
			/*
			printf("请输入密码:");
			char password[sizeof(user_in.password)];
			scanf("%s", &password);*/
			if (strncmp(args[2], user_in.password, strlen(args[2]))==0) {
				strcpy(active_user, args[1]);
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

int create_user(char** args) {
	if (strncmp(active_user, "root", 4)!=0) {
		return -1;
	}
	user new_user;
	new_user.is_system = 0;
	new_user.is_valid = 1;
	Time t = query_time();
	strcpy(new_user.establish_time, t.time);
	strcpy(new_user.establish_date, t.date);
	strcpy(new_user.user_name, args[2]);
	strcpy(new_user.password, args[3]);
	/* 寻找一个空闲的区间放入user */
	FILE* fp;
	fp = fopen("vdisk", "r+");
	fseek(fp, (long)((64 + sector_per_FAT_int * 2)*SECTOR), SEEK_SET);
	user user_in;
	for (int i = 0;i < user_num;i++) {
		fread(&user_in, sizeof(user_in), 1, fp);
		if (user_in.is_valid == 0) {
			fseek(fp, -(long)(sizeof(user_in)), SEEK_CUR);
			fwrite(&new_user, sizeof(new_user), 1, fp);
			break;
		}
	}
	fclose(fp);
	strcpy(active_user, new_user.user_name);
	return 0;
}

int delete_user() {

	return 0;
}