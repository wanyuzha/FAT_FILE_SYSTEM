#include "file.h"
#include <stdlib.h>
#include <string.h>

/* 指定当前存储该文件夹下所有信息的簇号 */
int dir_index = 2;
extern char active_user[];
int sector_per_cluster_int;

/* 声明它是一个外部变量，与user当中相同 */
extern int sector_per_FAT_int;

int file_param_initial() {
	FILE* fp;
	fp = fopen("vdisk", "rb");
	unsigned char sector_per_cluster[1];
	fseek(fp, (long)(SECTOR * 63 + 0x0d), SEEK_SET);
	fread(sector_per_cluster, sizeof(sector_per_cluster), 1, fp);
	sector_per_cluster_int = hex_char_dex(sector_per_cluster, sizeof(sector_per_cluster));
	sector_per_FAT_int = (10 * BUFFER_SIZE) / (SECTOR*(int)sector_per_cluster[0] * 1 << 8);
	fclose(fp);
	return 0;
}
/*
int show_dir() {
	FILE* fp;
	fp = fopen("vdisk", "rb");
	
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
*/
int create_file(char** filename) {
	/* 首先确定文件的各个属性，然后再按字节流读入文件 */
	catalog new_file;
	/* 如果没有指定属性，默认最高777 */
	strcpy(new_file.attribute, "777");
	char *args[ARGS_MAX_NUM] = { 0 };
	int num = 0;
	split(filename[2], ".", args, &num);
	if (num == 2) {
		strcpy(new_file.filename, args[0]);
		strcpy(new_file.filename_extension, args[1]);
		strcpy(new_file.user_name, active_user);
	}

	if (filename[3] != NULL) {
		strcpy(new_file.attribute, filename[3]);
	}
	new_file.is_valid = 1;
	new_file.file_size = 0;
	

	FILE* fp;
	fp = fopen("vdisk", "rb+");

	/* 查询FAT表，分配一个簇号，此时文件指针指向FAT表区域 */
	char cluster_index[2];
	not_allocate_FAT_index(fp, cluster_index, sector_per_FAT_int);
	string_copy(new_file.start_cluster, cluster_index, sizeof(cluster_index));

	/*	遍历目录表项，解决文件的重名问题 
		向文件名末尾添加时间信息
	*/
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	int catalog_num = sector_per_cluster_int * SECTOR / sizeof(catalog);
	for (int i = 0;i < catalog_num;i++) {
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		if (c.is_valid == 1) {
			if (strcmp(new_file.filename, c.filename)==0 && strcmp(new_file.filename_extension, c.filename_extension)==0 && strcmp(new_file.user_name, c.user_name)==0) {
				char time[10];
				time_to_char(time);
				strncat(new_file.filename, time, sizeof(time));
			}
		}
	}
	/* 解决重名问题后，找到一个空的位置，将其插入 */
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	for (int i = 0;i < catalog_num;i++) {
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		if (c.is_valid == 0) {
			fseek(fp, -(long)sizeof(c), SEEK_CUR);
			fwrite(&new_file, sizeof(new_file), 1, fp);
			break;
		}
	}
	fclose(fp);
	return 0;
}

int create_dir(char** filename) {
	/* 首先确定文件的各个属性，然后再按字节流读入文件 */
	catalog new_file;
	/* 如果没有指定属性，默认最高777 */
	strcpy(new_file.attribute, "777");
	strcpy(new_file.filename, filename[1]);
	strcpy(new_file.filename_extension, "dir");
	strcpy(new_file.user_name, active_user);

	if (filename[2] != NULL) {
		strcpy(new_file.attribute, filename[2]);
	}
	new_file.is_valid = 1;
	new_file.file_size = 0;
	new_file.former_cluster[0] = dir_index>>8;
	new_file.former_cluster[1] = dir_index;

	FILE* fp;
	fp = fopen("vdisk", "rb+");

	/* 查询FAT表，分配一个簇号，此时文件指针指向FAT表区域 */
	char cluster_index[2];
	not_allocate_FAT_index(fp, cluster_index, sector_per_FAT_int);
	string_copy(new_file.start_cluster, cluster_index, sizeof(cluster_index));
	int start_cluster_int = (cluster_index[0] << 8) + cluster_index[1];

	/*	遍历目录表项，解决文件的重名问题
	向文件名末尾添加时间信息
	*/
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	int catalog_num = sector_per_cluster_int * SECTOR / sizeof(catalog);
	for (int i = 0;i < catalog_num;i++) {
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		if (c.is_valid == 1) {
			if (strcmp(new_file.filename, c.filename) == 0 && strcmp(new_file.filename_extension, c.filename_extension) == 0 && strcmp(new_file.user_name, c.user_name) == 0) {
				char time[10];
				time_to_char(time);
				strncat(new_file.filename, time, sizeof(time));
			}
		}
	}

	/* 解决重名问题后，找到一个空的位置，将其插入 */
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	for (int i = 0;i < catalog_num;i++) {
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		if (c.is_valid != 1) {
			fseek(fp, -(long)sizeof(c), SEEK_CUR);
			fwrite(&new_file, sizeof(new_file), 1, fp);
			break;
		}
	}
	/* 将目录项放到新簇号的开头 */
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * start_cluster_int)*SECTOR), SEEK_SET);
	fwrite(&new_file, sizeof(new_file), 1, fp);
	fclose(fp);
	return 0;
}

/*	内部功能函数，返回目录项在簇中的编号，目前只能对同一文件夹下的文件操作 */
int find_file(char* filename, int& catalog_index) {
	/* 使用时请保证文件指针关闭 */
	char *args[ARGS_MAX_NUM] = { 0 };
	int num = 0;
	split(filename, "/", args, &num);
	if (num == 1) {
		/* 查找的是当前目录下的文件 */
		split(filename, ".", args, &num);
		FILE* fp;
		fp = fopen("vdisk", "rb");
		fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
		int catalog_num = sector_per_cluster_int * SECTOR / sizeof(catalog);
		if (num == 2) {
			for (int i = 0;i < catalog_num;i++) {
				catalog c;
				fread(&c, sizeof(c), 1, fp);
				if (c.is_valid == 1) {
					if (strncmp(c.filename, args[0], strlen(args[0])) == 0 && strncmp(c.filename_extension, args[1], strlen(args[1])) == 0) {
						catalog_index = i;
						/* 返回该目录下的索引号 */
						int index = (c.start_cluster[0] * 1 << 8) + c.start_cluster[1];
						fclose(fp);
						return index;
					}
				}
			}
		}
		else
		{
			for (int i = 0;i < catalog_num;i++) {
				catalog c;
				fread(&c, sizeof(c), 1, fp);
				if (c.is_valid == 1) {
					if (strncmp(c.filename, args[0], strlen(args[0])) == 0 ) {
						catalog_index = i;
						/* 返回该目录下的索引号 */
						int index = (c.start_cluster[0] * 1 << 8) + c.start_cluster[1];
						fclose(fp);
						return index;
					}
				}
			}
		}
		fclose(fp);
	}
	else {

	}
	return -1;
}

/*	内部功能函数，判断用户是否有权限 
	授权的同时返回具体的目录信息*/
int user_authorize(int catalog_index, catalog& c) {
	FILE* fp;
	fp = fopen("vdisk", "rb+");
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(c)), SEEK_SET);
	fread(&c, sizeof(c), 1, fp);
	if (strcmp(c.user_name, active_user) != 0) {
		if (c.attribute[3] < 4 + '0') {
			return -2;
		}
	}
	fclose(fp);
	return 1;
}

/*	路径解析函数：按照/解析各个路径,并最终返回catalog_index*/
long path_parsing(char* path) {
	char *args[ARGS_MAX_NUM] = { 0 };
	int num = 0;
	split(path, "/", args, &num);
	/* 对一段路径而言，除了最后为文件名，前面都是目录名，保存旧的目录index	*/
	/* 要求路径需要从头找起或者为当前文件夹 */
	int catalog_index;
	if (num == 1) {
		if (find_file(args[0], catalog_index) == -1) {
			return -1;
		}
		return (64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(catalog);
	}
	int old_dir_index = dir_index;
	if (strcmp(args[0], "root") != 0) {
		return -1;
	}
	dir_index = 2;
	for (int i = 1;i < num - 1;i++) {
		if (find_file(args[i], catalog_index) == -1) {
			return -1;
		}
		FILE* fp;
		fp = fopen("vdisk", "rb+");
		fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(catalog)), SEEK_SET);
		/* 开始寻找下一个簇 */
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		dir_index = (c.start_cluster[0] << 8) + c.start_cluster[1];
		fclose(fp);
	}
	if (find_file(args[num-1], catalog_index) == -1) {
		return -1;
	}
	long offset = (64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(catalog);
	dir_index = old_dir_index;
	return offset;
}

/*	移动函数 */
int move_file(char** args) {
	long offset = path_parsing(args[1]);
	if (offset == -1) {
		return -1;
	}
	FILE* fp;
	fp = fopen("vdisk", "rb+");
	fseek(fp, offset, SEEK_SET);
	catalog c;
	fread(&c, sizeof(c), 1, fp);
	c.is_valid = 0;
	fseek(fp, -(long)sizeof(c), SEEK_CUR);
	fwrite(&c, sizeof(c), 1, fp);
	c.is_valid = 1;

	offset = path_parsing(args[2]);
	if (offset == -1) {
		return -1;
	}
	fseek(fp, offset, SEEK_SET);
	catalog dest;
	fread(&dest, sizeof(dest), 1, fp);
	int old_dir_index = dir_index;
	dir_index = (dest.start_cluster[0] << 8) + dest.start_cluster[1];

	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	int catalog_num = sector_per_cluster_int * SECTOR / sizeof(catalog);
	for (int i = 0;i < catalog_num;i++) {
		catalog temp;
		fread(&temp, sizeof(catalog), 1, fp);
		if (temp.is_valid != 1) {
			fseek(fp, -(long)sizeof(catalog), SEEK_CUR);
			fwrite(&c, sizeof(c), 1, fp);
			break;
		}
	}
	fclose(fp);
	dir_index = old_dir_index;
	//path_parsing(args[2]);
	return 0;
}

/*  目前依然存在问题 先跳过 */
int write_file(char** filename) {
	/* 返回文件名的簇号 */
	int catalog_index = 0;
	int cluster_index = find_file(filename[1], catalog_index);
	if (cluster_index == -1) {
		return -1;
	}
	FILE* fp;
	fp = fopen("vdisk", "rb+");

	FILE *src;
	src = fopen(filename[2], "rb+");
	if (src == NULL) {
		return -1;
	}
	fseek(src, 0, SEEK_END);
	int filelength = ftell(src);
	catalog c;
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + catalog_index*sizeof(catalog)), SEEK_SET);
	long offset = (64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + catalog_index * sizeof(catalog);
	fread(&c, sizeof(c), 1, fp);
	c.file_size = filelength;
	/* 回退重填 */
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + catalog_index * sizeof(catalog)), SEEK_SET);
	fwrite(&c, sizeof(c), 1, fp);
	fflush(fp);
	
	fseek(src, 0, SEEK_SET);

	/* 控制buffer的长度恰好为一簇, 并初始化为全0 */
	char* buffer = (char*)malloc(SECTOR*sector_per_cluster_int);
	int i = 0;
	while (i<filelength)
	{
		/* buffer每轮清空 */
		for (int i = 0;i < SECTOR*sector_per_cluster_int;i++) {
			buffer[i] = 0;
		}
		if (filelength - i > SECTOR * sector_per_cluster_int)
		{
			fread(buffer, SECTOR*sector_per_cluster_int, 1, src);
		}
		else {
			fread(buffer, filelength - i, 1, src);
		}
		long offset = SECTOR * 64 + sector_per_FAT_int * 2 * SECTOR + sector_per_cluster_int * SECTOR*cluster_index;
		fseek(fp, offset, SEEK_SET);
		fwrite(buffer, SECTOR*sector_per_cluster_int, 1, fp);
		fflush(fp);
		/* 更新FAT表 */
		i += SECTOR * sector_per_cluster_int;
		if (i < filelength) {
			char cluster_index_char[2];
			not_allocate_FAT_index(fp, cluster_index_char, sector_per_FAT_int);
			fseek(fp, SECTOR * 64 + cluster_index * 2, SEEK_SET);
			fwrite(cluster_index_char, sizeof(cluster_index_char), 1, fp);
			cluster_index = (cluster_index_char[0] << 8) + cluster_index_char[1];
		}
	}
	fclose(fp);
	fclose(src);
	return 0;
}



int read_dir(char* filename) {
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
	if (strcmp(c.user_name, active_user) != 0) {
		if (c.attribute[3] < 4 + '0') {
			return -2;
		}
	}
	char cluster_index[2];
	string_copy(cluster_index, c.start_cluster, sizeof(cluster_index));
	int cluster_index_int = (cluster_index[0] << 8) + cluster_index[1];
	dir_index = cluster_index_int;
	fclose(fp);
	return 0;
}

int return_dir() {
	FILE* fp;
	fp = fopen("vdisk", "rb");

	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	catalog c;
	fread(&c, sizeof(catalog), 1, fp);
	dir_index = (c.former_cluster[0] << 8) + c.former_cluster[1];
	return 0;
}

/* 文件或文件夹改名 */
int rename(char* filename, char* newname) {
	int catalog_index;
	find_file(filename, catalog_index);
	if (catalog_index == -1) {
		return -1;
	}
	FILE* fp;
	fp = fopen("vdisk", "rb+");
	catalog c;
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(c)), SEEK_SET);
	fread(&c, sizeof(c), 1, fp);
	if (strcmp(c.user_name, active_user) != 0) {
		if ((c.attribute[3]-'0')%2==0) {
			return -2;
		}
	}
	char *args[ARGS_MAX_NUM] = { 0 };
	int num = 0;
	split(newname, ".", args, &num);
	strcpy(c.filename, args[0]);
	strcpy(c.filename_extension, args[1]);
	fseek(fp, -(long)sizeof(catalog), SEEK_CUR);
	fwrite(&c, sizeof(c), 1, fp);
	fclose(fp);
	return 0;
}

/* 删除文件 */
int delete_file(char* filename) {
	int catalog_index;
	find_file(filename, catalog_index);
	if (catalog_index == -1) {
		return -1;
	}
	catalog c;
	if (!user_authorize(catalog_index, c)) {
		return -2;
	}
	FILE* fp;
	fp = fopen("vdisk", "rb+");

	c.is_valid = 0;
	fseek(fp, ((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(c)), SEEK_SET);
	fwrite(&c, sizeof(c), 1, fp);
	fflush(fp);
	int cluster_index = (c.start_cluster[0] << 8) + c.start_cluster[1];
	fseek(fp, 64 * SECTOR + cluster_index * 2, SEEK_SET);
	char cluster_index_char[2];
	char unuse[2] = { 0, 0 };
	fread(cluster_index_char, sizeof(cluster_index_char), 1, fp);
	while (cluster_index_char[0]!=-1&&cluster_index_char[1]!=-1)
	{
		fseek(fp, -(long)sizeof(cluster_index_char), SEEK_CUR);
		fwrite(unuse, sizeof(unuse), 1, fp);
		cluster_index = (cluster_index_char[0] << 8) + cluster_index_char[1];
		fseek(fp, 64 * SECTOR + cluster_index * 2, SEEK_SET);
		fread(cluster_index_char, sizeof(cluster_index_char), 1, fp);
	}
	fseek(fp, -(long)sizeof(cluster_index_char), SEEK_CUR);
	fwrite(unuse, sizeof(unuse), 1, fp);
	fflush(fp);
	fclose(fp);
	return 0;
}

/*	删除文件夹可以转换为删除文件 
	目前文件夹要求输入后缀名*/
int delete_dir(char* filename) {
	int catalog_index;
	find_file(filename, catalog_index);
	if (catalog_index == -1) {
		return -1;
	}
	catalog catalog_in;
	if (!user_authorize(catalog_index, catalog_in)) {
		return -2;
	}
	delete_file(filename);
	FILE* fp;
	fp = fopen("vdisk", "rb");
	int old_dir_index = dir_index;
	dir_index = (catalog_in.start_cluster[0] << 8) + catalog_in.start_cluster[1];
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR), SEEK_SET);
	int catalog_num = sector_per_cluster_int * SECTOR / sizeof(catalog);
	for (int i = 0;i < catalog_num;i++) {
		catalog c;
		fread(&c, sizeof(c), 1, fp);
		if (c.is_valid == 1) {
			char filename_temp[sizeof(c.filename)+sizeof(c.filename_extension)];
			strncpy(filename_temp, c.filename, sizeof(c.filename));
			strncat(filename_temp, ".", 2);
			strncat(filename_temp, c.filename_extension, sizeof(c.filename_extension));
			fclose(fp);
			delete_file(filename_temp);
			fp = fopen("vdisk", "rb");
		}
	}
	dir_index = old_dir_index;
	fclose(fp);
	return 0;
}

/* struct拷贝 */
int struct_copy(catalog &dest, catalog src) {
	strcpy(dest.attribute, src.attribute);
	strcpy(dest.establish_date, src.establish_date);
	strcpy(dest.establish_time, src.establish_time);
	strcpy(dest.filename, src.filename);
	strcpy(dest.filename_extension, src.filename_extension);
	strcpy(dest.former_cluster, src.former_cluster);
	strcpy(dest.latest_access_date, src.latest_access_date);
	strcpy(dest.modify_date, src.modify_date);
	strcpy(dest.modify_time, src.modify_time);
	strcpy(dest.start_cluster, src.start_cluster);
	dest.is_valid = src.is_valid;
	dest.file_size = src.file_size;
	dest.establish_time_precise = src.establish_time_precise;
	return 0;
}

/* 使用以下程序来进行测试 */
int test_file(char** filename) {
	/* 返回文件名的簇号 */
	int catalog_index = 0;
	int cluster_index = find_file(filename[1], catalog_index);
	if (cluster_index == -1) {
		printf("您输入的文件名有误，请重新输入");
		return -1;
	}
	FILE* fp;
	fp = fopen("vdisk", "rb");

	FILE *src;
	src = fopen(filename[2], "rb");
	if (src == NULL) {
		printf("您输入的文件名有误，请重新输入");
		return -1;
	}
	fseek(src, 0, SEEK_END);
	int filelength = ftell(src);
	catalog c;
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + catalog_index * sizeof(catalog)), SEEK_SET);
	long offset = (64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + catalog_index * sizeof(catalog);
	fread(&c, sizeof(c), 1, fp);
	catalog c_new;
	c.file_size = filelength;
	struct_copy(c_new, c);
	/* 回退重填 */
	
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + catalog_index * sizeof(catalog)), SEEK_SET);
	int result = fwrite(&c_new, sizeof(catalog), 1, fp);
	fflush(fp);
	fseek(fp, (long)((64 + sector_per_FAT_int * 2 + sector_per_cluster_int * dir_index)*SECTOR + (catalog_index) * sizeof(catalog)), SEEK_SET);
	fread(&c, sizeof(catalog), 1, fp);

	fseek(src, 0, SEEK_SET);

	
	fclose(fp);
	fclose(src);
	return 0;
}